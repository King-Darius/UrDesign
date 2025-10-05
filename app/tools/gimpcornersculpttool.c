/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpcornersculpttool.c
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

#include "tools-types.h"

#include "core/gimpimage.h"
#include "core/gimpimage-undo-push.h"
#include "core/gimpitem.h"

#include "path/gimpanchor.h"
#include "path/gimppath.h"
#include "path/gimpstroke.h"

#include "widgets/gimphelp-ids.h"

#include "gimpcornersculptoptions.h"
#include "gimpcornersculpttool.h"

#include "gimp-intl.h"


G_DEFINE_TYPE (GimpCornerSculptTool, gimp_corner_sculpt_tool,
               GIMP_TYPE_PATH_TOOL)

typedef struct
{
  GimpStroke *stroke;
  gint        anchor_index;
} GimpCornerSculptTarget;

static void   gimp_corner_sculpt_tool_constructed  (GObject            *object);
static void   gimp_corner_sculpt_tool_dispose      (GObject            *object);
static void   gimp_corner_sculpt_tool_options_notify
                                                   (GObject            *options,
                                                    GParamSpec         *pspec,
                                                    gpointer            data);
static void   gimp_corner_sculpt_tool_apply        (GimpCornerSculptTool *tool);
static void   gimp_corner_sculpt_tool_targets_free (GArray             *targets);


void
gimp_corner_sculpt_tool_register (GimpToolRegisterCallback callback,
                                  gpointer                 data)
{
  (* callback) (GIMP_TYPE_CORNER_SCULPT_TOOL,
                GIMP_TYPE_CORNER_SCULPT_OPTIONS,
                gimp_corner_sculpt_options_gui,
                GIMP_CONTEXT_PROP_MASK_NONE,
                "gimp-corner-sculpt-tool",
                _("Corner Sculpt"),
                _("Corner Sculpt Tool: Refine and reshape vector corners"),
                N_("Corner _Sculpt"), NULL,
                NULL, GIMP_HELP_TOOL_CORNER_SCULPT,
                GIMP_ICON_TOOL_CORNER_SCULPT,
                data);
}

static void
gimp_corner_sculpt_tool_class_init (GimpCornerSculptToolClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = gimp_corner_sculpt_tool_constructed;
  object_class->dispose     = gimp_corner_sculpt_tool_dispose;
}

static void
gimp_corner_sculpt_tool_init (GimpCornerSculptTool *tool)
{
  GimpTool *gimp_tool = GIMP_TOOL (tool);

  tool->mode_notify_id   = 0;
  tool->radius_notify_id = 0;

  gimp_tool_control_set_tool_cursor (gimp_tool->control,
                                     GIMP_TOOL_CURSOR_PATHS);
  gimp_tool_control_set_precision (gimp_tool->control,
                                   GIMP_CURSOR_PRECISION_SUBPIXEL);
}

static void
gimp_corner_sculpt_tool_constructed (GObject *object)
{
  GimpCornerSculptTool    *tool    = GIMP_CORNER_SCULPT_TOOL (object);
  GimpTool                *gimp_tool = GIMP_TOOL (tool);
  GimpToolOptions         *tool_options;
  GObject                 *options_object;

  G_OBJECT_CLASS (gimp_corner_sculpt_tool_parent_class)->constructed (object);

  tool_options = gimp_tool_get_options (gimp_tool);
  options_object = tool_options ? G_OBJECT (tool_options) : NULL;

  if (options_object)
    {
      tool->mode_notify_id =
        g_signal_connect (options_object,
                          "notify::corner-mode",
                          G_CALLBACK (gimp_corner_sculpt_tool_options_notify),
                          tool);
      tool->radius_notify_id =
        g_signal_connect (options_object,
                          "notify::corner-radius",
                          G_CALLBACK (gimp_corner_sculpt_tool_options_notify),
                          tool);
    }
}

static void
gimp_corner_sculpt_tool_dispose (GObject *object)
{
  GimpCornerSculptTool *tool = GIMP_CORNER_SCULPT_TOOL (object);
  GimpToolOptions      *tool_options;

  tool_options = gimp_tool_get_options (GIMP_TOOL (tool));

  if (tool_options)
    {
      GObject *options_object = G_OBJECT (tool_options);

      if (tool->mode_notify_id)
        {
          g_signal_handler_disconnect (options_object, tool->mode_notify_id);
          tool->mode_notify_id = 0;
        }

      if (tool->radius_notify_id)
        {
          g_signal_handler_disconnect (options_object, tool->radius_notify_id);
          tool->radius_notify_id = 0;
        }
    }

  G_OBJECT_CLASS (gimp_corner_sculpt_tool_parent_class)->dispose (object);
}

static void
gimp_corner_sculpt_tool_options_notify (GObject    *options,
                                        GParamSpec *pspec,
                                        gpointer    data)
{
  GimpCornerSculptTool *tool = GIMP_CORNER_SCULPT_TOOL (data);

  gimp_corner_sculpt_tool_apply (tool);
}

static void
gimp_corner_sculpt_tool_apply (GimpCornerSculptTool *tool)
{
  GimpPathTool              *path_tool = GIMP_PATH_TOOL (tool);
  GimpCornerSculptOptions   *options;
  GimpPath                  *path;
  GArray                    *targets;
  GimpStroke                *stroke = NULL;
  gboolean                   changed = FALSE;

  g_return_if_fail (GIMP_IS_CORNER_SCULPT_TOOL (tool));

  options = GIMP_CORNER_SCULPT_OPTIONS (gimp_tool_get_options (GIMP_TOOL (tool)));
  path    = path_tool->path;

  if (! options || ! path)
    return;

  targets = g_array_new (FALSE, FALSE, sizeof (GimpCornerSculptTarget));

  while ((stroke = gimp_path_stroke_get_next (path, stroke)))
    {
      GList *link;
      gint   anchor_index = 0;

      for (link = stroke->anchors->head; link; link = link->next)
        {
          GimpAnchor *anchor = link->data;

          if (anchor->type != GIMP_ANCHOR_ANCHOR)
            continue;

          if (anchor->selected)
            {
              GimpCornerSculptTarget target = { g_object_ref (stroke), anchor_index };
              g_array_append_val (targets, target);
            }

          anchor_index++;
        }
    }

  if (targets->len == 0)
    {
      gimp_corner_sculpt_tool_targets_free (targets);
      return;
    }

  {
    GimpImage *image = GIMP_IMAGE (gimp_item_get_image (GIMP_ITEM (path)));

    if (image)
      {
        guint i;
        gdouble radius = MAX (0.0, options->corner_radius);

        gimp_image_undo_push_path_mod (image,
                                       C_("undo-type", "Sculpt corners"),
                                       path);

        gimp_path_freeze (path);

        for (i = 0; i < targets->len; i++)
          {
            GimpCornerSculptTarget target =
              g_array_index (targets, GimpCornerSculptTarget, i);

            gimp_stroke_corner_set (target.stroke,
                                     target.anchor_index,
                                     options->corner_mode,
                                     radius);
            changed = TRUE;
          }

        gimp_path_thaw (path);

        if (changed)
          gimp_image_flush (image);
      }
  }

  gimp_corner_sculpt_tool_targets_free (targets);
}

static void
gimp_corner_sculpt_tool_targets_free (GArray *targets)
{
  guint i;

  if (! targets)
    return;

  for (i = 0; i < targets->len; i++)
    {
      GimpCornerSculptTarget target =
        g_array_index (targets, GimpCornerSculptTarget, i);

      g_object_unref (target.stroke);
    }

  g_array_free (targets, TRUE);
}
