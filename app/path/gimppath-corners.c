/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimppath-corners.c
 * Copyright (C) 2024 UrDesign contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <math.h>

#include "libgimpmath/gimpmath.h"

#include "path-types.h"

#include "core/gimpimage.h"
#include "core/gimpimage-undo.h"
#include "core/gimpintl.h"

#include "gimpanchor.h"
#include "gimppath.h"
#include "gimppath-corners.h"
#include "gimppathmodundo.h"
#include "gimpstroke.h"
#include "gimpstroke-new.h"

#define EPSILON 1e-6

typedef struct
{
  GimpCoords point;
  GimpCoords entry;
  GimpCoords exit;
  GimpCoords handle1;
  GimpCoords handle2;
  gboolean   has_entry;
  gboolean   has_exit;
  gboolean   use_round;
  gboolean   touched;
  gdouble    radius;
} CornerData;

static inline GimpCoords
coords_init (gdouble x,
             gdouble y)
{
  GimpCoords coords = GIMP_COORDS_DEFAULT_VALUES;

  coords.x = x;
  coords.y = y;

  return coords;
}

static inline GimpCoords
coords_add_scaled (const GimpCoords *a,
                   const GimpCoords *b,
                   gdouble           scale)
{
  return coords_init (a->x + b->x * scale,
                      a->y + b->y * scale);
}

static inline GimpCoords
coords_diff (const GimpCoords *a,
             const GimpCoords *b)
{
  return coords_init (a->x - b->x,
                      a->y - b->y);
}

static inline gdouble
coords_length (const GimpCoords *a)
{
  return hypot (a->x, a->y);
}

static gboolean
coords_normalize (GimpCoords *coords)
{
  gdouble len = coords_length (coords);

  if (len < EPSILON)
    return FALSE;

  coords->x /= len;
  coords->y /= len;

  return TRUE;
}

static inline gdouble
coords_dot (const GimpCoords *a,
            const GimpCoords *b)
{
  return a->x * b->x + a->y * b->y;
}

static inline gdouble
coords_cross (const GimpCoords *a,
              const GimpCoords *b)
{
  return a->x * b->y - a->y * b->x;
}

static inline GimpCoords
coords_rotate_90 (const GimpCoords *v,
                  gdouble           orientation)
{
  if (orientation >= 0.0)
    return coords_init (-v->y, v->x);

  return coords_init (v->y, -v->x);
}

static gboolean
coords_close (const GimpCoords *a,
              const GimpCoords *b)
{
  return coords_length (&coords_diff (a, b)) < 1e-4;
}

static void
corner_sculpt_replace_stroke (GimpStroke *target,
                              GimpStroke *source)
{
  GList *list;

  while (! g_queue_is_empty (target->anchors))
    {
      GimpAnchor *anchor = g_queue_pop_head (target->anchors);

      gimp_anchor_free (anchor);
    }

  for (list = source->anchors->head; list; list = g_list_next (list))
    g_queue_push_tail (target->anchors,
                       gimp_anchor_copy (list->data));

  target->closed = source->closed;
}

static GimpStroke *
corner_sculpt_build_stroke (GimpStroke           *stroke,
                            gdouble               radius,
                            GimpCornerSculptMode  mode,
                            gboolean             *changed)
{
  GArray     *anchors;
  GArray     *nodes;
  gboolean    closed = FALSE;
  gint        n_nodes;
  CornerData *corner_data;
  gboolean    touched = FALSE;
  gint        i;

  anchors = gimp_stroke_control_points_get (stroke, &closed);
  if (! anchors)
    return NULL;

  nodes = g_array_new (FALSE, FALSE, sizeof (GimpCoords));

  for (i = 0; i < anchors->len; i++)
    {
      GimpAnchor *anchor = &g_array_index (anchors, GimpAnchor, i);

      if (anchor->type == GIMP_ANCHOR_ANCHOR)
        g_array_append_val (nodes, anchor->position);
    }

  g_array_unref (anchors);

  n_nodes = nodes->len;

  if (n_nodes < (closed ? 3 : 2))
    {
      g_array_unref (nodes);
      return NULL;
    }

  corner_data = g_new0 (CornerData, n_nodes);

  for (i = 0; i < n_nodes; i++)
    {
      GimpCoords point = g_array_index (nodes, GimpCoords, i);
      gint       prev  = (i - 1 + n_nodes) % n_nodes;
      gint       next  = (i + 1) % n_nodes;
      GimpCoords prev_point;
      GimpCoords next_point;
      GimpCoords dir_prev;
      GimpCoords dir_next;
      GimpCoords away_prev;
      GimpCoords away_next;
      gdouble    len_prev;
      gdouble    len_next;
      gdouble    dot;
      gdouble    angle;
      gdouble    tan_half;
      gdouble    max_radius;
      gdouble    actual_radius;
      gdouble    offset;
      gdouble    sin_half;
      GimpCoords bisector;
      gdouble    bis_len;
      GimpCoords center;
      GimpCoords radial_entry;
      GimpCoords radial_exit;
      gdouble    handle_len;
      gdouble    orientation;

      corner_data[i].point = point;

      if (! closed && (i == 0 || i == n_nodes - 1))
        continue;

      prev_point = g_array_index (nodes, GimpCoords, prev);
      next_point = g_array_index (nodes, GimpCoords, next);

      dir_prev = coords_diff (&point, &prev_point);
      dir_next = coords_diff (&next_point, &point);

      len_prev = coords_length (&dir_prev);
      len_next = coords_length (&dir_next);

      if (len_prev < EPSILON || len_next < EPSILON)
        continue;

      coords_normalize (&dir_prev);
      coords_normalize (&dir_next);

      away_prev = coords_init (-dir_prev.x, -dir_prev.y);
      away_next = dir_next;

      dot = CLAMP (coords_dot (&away_prev, &away_next), -0.999999, 0.999999);
      angle = acos (dot);

      if (angle <= 1e-3 || angle >= (G_PI - 1e-3))
        continue;

      tan_half = tan (angle / 2.0);

      if (fabs (tan_half) < 1e-6)
        continue;

      max_radius = MIN (len_prev, len_next) * tan_half;
      actual_radius = MIN (radius, max_radius);

      if (actual_radius <= 0.0)
        continue;

      offset = actual_radius / tan_half;

      if (offset <= 0.0 || offset > len_prev || offset > len_next)
        continue;

      corner_data[i].has_entry = TRUE;
      corner_data[i].has_exit  = TRUE;
      corner_data[i].use_round = (mode == GIMP_CORNER_SCULPT_MODE_ROUND);
      corner_data[i].radius    = actual_radius;

      corner_data[i].entry = coords_add_scaled (&point, &dir_prev, -offset);
      corner_data[i].exit  = coords_add_scaled (&point, &dir_next, offset);

      sin_half = sin (angle / 2.0);

      if (fabs (sin_half) < 1e-6)
        sin_half = (sin_half < 0.0) ? -1e-6 : 1e-6;

      bisector = coords_init (away_prev.x + away_next.x,
                              away_prev.y + away_next.y);
      bis_len = coords_length (&bisector);

      if (bis_len < EPSILON)
        continue;

      bisector.x /= bis_len;
      bisector.y /= bis_len;

      center = coords_add_scaled (&point, &bisector,
                                  actual_radius / sin_half);

      radial_entry = coords_diff (&corner_data[i].entry, &center);
      radial_exit  = coords_diff (&corner_data[i].exit,  &center);

      coords_normalize (&radial_entry);
      coords_normalize (&radial_exit);

      orientation = coords_cross (&away_prev, &away_next);

      if (corner_data[i].use_round)
        {
          GimpCoords tangent_entry;
          GimpCoords tangent_exit;

          tangent_entry = coords_rotate_90 (&radial_entry, orientation);
          tangent_exit  = coords_rotate_90 (&radial_exit,  orientation);

          handle_len = actual_radius * (4.0 / 3.0) * tan (angle / 4.0);

          corner_data[i].handle1 = coords_add_scaled (&corner_data[i].entry,
                                                      &tangent_entry,
                                                      handle_len);
          corner_data[i].handle2 = coords_add_scaled (&corner_data[i].exit,
                                                      &tangent_exit,
                                                      -handle_len);
        }

      corner_data[i].touched = TRUE;
      touched = TRUE;
    }

  if (! touched)
    {
      g_free (corner_data);
      g_array_unref (nodes);
      return NULL;
    }

  {
    GimpCoords start = corner_data[0].has_entry ? corner_data[0].entry
                                                : corner_data[0].point;
    GimpCoords current = start;
    GimpStroke *result = gimp_bezier_stroke_new_moveto (&start);
    gint        last_index = closed ? n_nodes : n_nodes - 1;

    for (i = 0; i < last_index; i++)
      {
        gint       next_index = (i + 1) % n_nodes;
        CornerData *next_data = &corner_data[next_index];
        GimpCoords  target = next_data->has_entry ? next_data->entry
                                                  : next_data->point;

        if (! coords_close (&current, &target))
          {
            gimp_bezier_stroke_lineto (result, &target);
            current = target;
          }

        if (next_data->has_entry && next_data->has_exit)
          {
            if (next_data->use_round)
              {
                if (! coords_close (&current, &next_data->exit))
                  {
                    gimp_bezier_stroke_cubicto (result,
                                                &next_data->handle1,
                                                &next_data->handle2,
                                                &next_data->exit);
                    current = next_data->exit;
                  }
              }
            else
              {
                if (! coords_close (&current, &next_data->exit))
                  {
                    gimp_bezier_stroke_lineto (result,
                                                &next_data->exit);
                    current = next_data->exit;
                  }
              }
          }
        else if (! coords_close (&current, &next_data->point))
          {
            gimp_bezier_stroke_lineto (result, &next_data->point);
            current = next_data->point;
          }
      }

    if (closed)
      gimp_stroke_close (result);

    g_free (corner_data);
    g_array_unref (nodes);

    *changed = TRUE;
    return result;
  }
}

gboolean
gimp_path_apply_corner_profile (GimpPath             *path,
                                gdouble               radius,
                                GimpCornerSculptMode  mode)
{
  GimpStroke *stroke;
  gboolean    changed = FALSE;
  gboolean    pushed_undo = FALSE;

  g_return_val_if_fail (GIMP_IS_PATH (path), FALSE);

  for (stroke = gimp_path_stroke_get_next (path, NULL);
       stroke;
       stroke = gimp_path_stroke_get_next (path, stroke))
    {
      gboolean    stroke_changed = FALSE;
      GimpStroke *replacement;

      replacement = corner_sculpt_build_stroke (stroke, radius, mode,
                                                &stroke_changed);

      if (stroke_changed && replacement)
        {
          if (! pushed_undo)
            {
              GimpImage *image = gimp_item_get_image (GIMP_ITEM (path));

              if (image)
                gimp_image_undo_push_path_mod (image,
                                               _("Corner sculpt"),
                                               path);

              gimp_path_freeze (path);
              pushed_undo = TRUE;
            }

          corner_sculpt_replace_stroke (stroke, replacement);
          changed = TRUE;
        }

      if (replacement)
        g_object_unref (replacement);
    }

  if (pushed_undo)
    gimp_path_thaw (path);

  return changed;
}
