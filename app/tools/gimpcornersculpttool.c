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

#include <gegl.h>

#include "libgimpconfig/gimpconfig.h"

#include "tools-types.h"

#include "core/gimpcontext.h"
#include "core/gimpimage.h"
#include "core/gimpvectors.h"

#include "display/gimpdisplay.h"

#include "path/gimppath.h"
#include "path/gimppath-corners.h"

#include "gimpcornersculptoptions.h"
#include "gimpcornersculpttool.h"
#include "gimptoolcontrol.h"

#include "gimp-intl.h"


static void     gimp_corner_sculpt_tool_button_press (GimpTool              *tool,
                                                      const GimpCoords      *coords,
                                                      guint32                time,
                                                      GdkModifierType        state,
                                                      GimpButtonPressType    press_type,
                                                      GimpDisplay           *display);

static void     gimp_corner_sculpt_tool_register_type (GType                 tool_type,
                                                       GType                 options_type,
                                                       GimpToolOptionsGUIFunc gui_func,
                                                       GimpContextPropMask   context_props,
                                                       const gchar          *identifier,
                                                       const gchar          *label,
                                                       const gchar          *tooltip,
                                                       const gchar          *menu_label,
                                                       const gchar          *menu_accel,
                                                       const gchar          *help_domain,
                                                       const gchar          *help_id,
                                                       const gchar          *icon_name,
                                                       gpointer              data);

G_DEFINE_TYPE (GimpCornerSculptTool, gimp_corner_sculpt_tool,
               GIMP_TYPE_DRAW_TOOL)

static void
gimp_corner_sculpt_tool_class_init (GimpCornerSculptToolClass *klass)
{
  GimpToolClass *tool_class = GIMP_TOOL_CLASS (klass);

  tool_class->button_press = gimp_corner_sculpt_tool_button_press;
}

static void
gimp_corner_sculpt_tool_init (GimpCornerSculptTool *tool)
{
  gimp_tool_control_set_tool_cursor (GIMP_TOOL (tool)->control,
                                     GIMP_TOOL_CURSOR_PATHS);
  gimp_tool_control_set_wants_click (GIMP_TOOL (tool)->control, TRUE);
  gimp_tool_control_set_wants_double_click (GIMP_TOOL (tool)->control, FALSE);
  gimp_tool_control_set_snap_to (GIMP_TOOL (tool)->control, TRUE);
  gimp_tool_control_set_precision (GIMP_TOOL (tool)->control,
                                   GIMP_CURSOR_PRECISION_SUBPIXEL);
}

static void
gimp_corner_sculpt_tool_button_press (GimpTool              *tool,
                                      const GimpCoords      *coords,
                                      guint32                time,
                                      GdkModifierType        state,
                                      GimpButtonPressType    press_type,
                                      GimpDisplay           *display)
{
  GimpCornerSculptOptions *options;
  GimpContext             *context;
  GimpImage               *image;
  GimpPath                *path;

  if (press_type != GIMP_BUTTON_PRESS_NORMAL)
    return;

  options = GIMP_CORNER_SCULPT_OPTIONS (tool->options);
  context = gimp_tool_get_context (tool);
  image   = gimp_display_get_image (display);

  if (! image)
    return;

  path = gimp_context_get_vectors (context);

  if (! path)
    path = gimp_image_pick_path (image, coords->x, coords->y, 6.0, 6.0);

  if (! path)
    {
      gimp_tool_message (tool, display,
                         _("Select or pick a path to sculpt."));
      return;
    }

  if (! gimp_path_apply_corner_profile (path,
                                         options->radius,
                                         options->mode))
    {
      gimp_tool_message (tool, display,
                         _("No corners were adjusted."));
      return;
    }

  gimp_image_flush (image);
}

static void
gimp_corner_sculpt_tool_register_type (GType                  tool_type,
                                       GType                  options_type,
                                       GimpToolOptionsGUIFunc options_gui_func,
                                       GimpContextPropMask    context_props,
                                       const gchar           *identifier,
                                       const gchar           *label,
                                       const gchar           *tooltip,
                                       const gchar           *menu_label,
                                       const gchar           *menu_accel,
                                       const gchar           *help_domain,
                                       const gchar           *help_id,
                                       const gchar           *icon_name,
                                       gpointer               data)
{
  GimpToolRegisterCallback callback = (GimpToolRegisterCallback) data;

  callback (tool_type,
            options_type,
            options_gui_func,
            context_props,
            identifier,
            label,
            tooltip,
            menu_label,
            menu_accel,
            help_domain,
            help_id,
            icon_name,
            NULL);
}

void
gimp_corner_sculpt_tool_register (GimpToolRegisterCallback callback,
                                  gpointer                 data)
{
  gimp_corner_sculpt_tool_register_type (GIMP_TYPE_CORNER_SCULPT_TOOL,
                                         GIMP_TYPE_CORNER_SCULPT_OPTIONS,
                                         gimp_corner_sculpt_options_gui,
                                         0,
                                         "gimp-corner-sculpt-tool",
                                         _("Corner Sculpt"),
                                         _("Corner Sculpt Tool"),
                                         N_("_Corner Sculpt"), NULL,
                                         GIMP_HELP_TOOL_CORNER_SCULPT,
                                         GIMP_ICON_TOOL_PATH,
                                         callback);
}
