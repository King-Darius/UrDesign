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

#include "widgets/gimphelp-ids.h"

#include "gimpcornersculptoptions.h"
#include "gimpcornersculpttool.h"

#include "gimp-intl.h"


G_DEFINE_TYPE (GimpCornerSculptTool, gimp_corner_sculpt_tool,
               GIMP_TYPE_PATH_TOOL)


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
}

static void
gimp_corner_sculpt_tool_init (GimpCornerSculptTool *tool)
{
  GimpTool *gimp_tool = GIMP_TOOL (tool);

  gimp_tool_control_set_tool_cursor (gimp_tool->control,
                                     GIMP_TOOL_CURSOR_PATHS);
  gimp_tool_control_set_precision (gimp_tool->control,
                                   GIMP_CURSOR_PRECISION_SUBPIXEL);
}
