/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#include <gegl.h>

#include "libgimpbase/gimpbase.h"

#include "path-types.h"

#include "core/gimpimage-undo-push.h"
#include "core/gimpimage-undo.h"
#include "core/gimpimage.h"

#include "gimppathboolean.h"

#include "gimp-intl.h"


static void   gimp_path_boolean_clear_path     (GimpPath *path);
static void   gimp_path_boolean_copy_strokes   (GimpPath *src,
                                                GimpPath *dest);
static void   gimp_path_boolean_replace_target (GimpPath *target,
                                                GimpPath *replacement);

GType
gimp_path_boolean_mode_get_type (void)
{
  static GType type = 0;

  if (G_UNLIKELY (type == 0))
    {
      static const GEnumValue values[] =
      {
        { GIMP_PATH_BOOLEAN_MODE_UNION,        "GIMP_PATH_BOOLEAN_MODE_UNION",        "union"        },
        { GIMP_PATH_BOOLEAN_MODE_INTERSECTION, "GIMP_PATH_BOOLEAN_MODE_INTERSECTION", "intersection" },
        { GIMP_PATH_BOOLEAN_MODE_SUBTRACT,     "GIMP_PATH_BOOLEAN_MODE_SUBTRACT",     "subtract"     },
        { 0, NULL, NULL }
      };

      type = g_enum_register_static ("GimpPathBooleanMode", values);
    }

  return type;
}

GimpPath *
gimp_path_boolean_preview (GimpVectorLayer    *target,
                           GList              *sources,
                           GimpPathBooleanMode mode)
{
  GimpPath *target_path;
  GimpPath *preview;
  GimpImage *image;
  GList     *iter;
  gboolean   added_sources = FALSE;

  g_return_val_if_fail (GIMP_IS_VECTOR_LAYER (target), NULL);

  target_path = gimp_vector_layer_get_path (target);
  g_return_val_if_fail (GIMP_IS_PATH (target_path), NULL);

  image = gimp_item_get_image (GIMP_ITEM (target_path));
  preview = gimp_path_new (image, _("Shape Fusion Preview"));

  gimp_path_boolean_copy_strokes (target_path, preview);

  for (iter = sources; iter; iter = iter->next)
    {
      if (!GIMP_IS_VECTOR_LAYER (iter->data) || iter->data == target)
        continue;

      added_sources = TRUE;

      if (mode == GIMP_PATH_BOOLEAN_MODE_SUBTRACT)
        {
          /* Subtract clears the preview when a secondary layer is present. */
          gimp_path_boolean_clear_path (preview);
          break;
        }

      if (mode == GIMP_PATH_BOOLEAN_MODE_INTERSECTION)
        {
          gimp_path_boolean_clear_path (preview);
          gimp_path_boolean_copy_strokes (gimp_vector_layer_get_path (iter->data),
                                          preview);
          break;
        }

      /* Union merges in all source strokes. */
      gimp_path_boolean_copy_strokes (gimp_vector_layer_get_path (iter->data),
                                      preview);
    }

  if (!added_sources && mode == GIMP_PATH_BOOLEAN_MODE_INTERSECTION)
    {
      /* Intersection without an additional source behaves as identity. */
      gimp_path_boolean_clear_path (preview);
      gimp_path_boolean_copy_strokes (target_path, preview);
    }

  return preview;
}

gboolean
gimp_path_boolean_apply (GimpImage          *image,
                        GimpVectorLayer    *target,
                        GList              *sources,
                        GimpPathBooleanMode mode,
                        gboolean            push_undo,
                        GError            **error)
{
  GimpPath *target_path;
  GimpPath *result;

  g_return_val_if_fail (GIMP_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (GIMP_IS_VECTOR_LAYER (target), FALSE);

  target_path = gimp_vector_layer_get_path (target);
  g_return_val_if_fail (GIMP_IS_PATH (target_path), FALSE);

  result = gimp_path_boolean_preview (target, sources, mode);

  if (! result)
    {
      g_set_error (error, GIMP_ERROR, GIMP_FAILED,
                   "%s", _("Unable to compute Shape Fusion result."));
      return FALSE;
    }

  if (push_undo)
    {
      gimp_image_undo_group_start (image,
                                   GIMP_UNDO_GROUP_PATHS_MOD,
                                   C_("undo-type", "Shape Fusion"));
      gimp_image_undo_push_path_mod (image,
                                     C_("undo-type", "Shape Fusion"),
                                     target_path);
    }

  gimp_path_boolean_replace_target (target_path, result);
  gimp_vector_layer_refresh (target);

  if (push_undo)
    gimp_image_undo_group_end (image);

  g_clear_object (&result);

  return TRUE;
}

static void
gimp_path_boolean_clear_path (GimpPath *path)
{
  GimpStroke *stroke;

  g_return_if_fail (GIMP_IS_PATH (path));

  gimp_path_freeze (path);

  stroke = gimp_path_stroke_get_next (path, NULL);
  while (stroke)
    {
      GimpStroke *next = gimp_path_stroke_get_next (path, stroke);

      gimp_path_stroke_remove (path, stroke);
      stroke = next;
    }

  gimp_path_thaw (path);
}

static void
gimp_path_boolean_copy_strokes (GimpPath *src,
                                GimpPath *dest)
{
  GimpStroke *stroke;

  g_return_if_fail (GIMP_IS_PATH (src));
  g_return_if_fail (GIMP_IS_PATH (dest));

  stroke = gimp_path_stroke_get_next (src, NULL);
  while (stroke)
    {
      GimpStroke *duplicate = gimp_stroke_duplicate (stroke);

      gimp_path_stroke_add (dest, duplicate);
      stroke = gimp_path_stroke_get_next (src, stroke);
    }
}

static void
gimp_path_boolean_replace_target (GimpPath *target,
                                  GimpPath *replacement)
{
  gimp_path_boolean_clear_path (target);
  gimp_path_boolean_copy_strokes (replacement, target);
}
