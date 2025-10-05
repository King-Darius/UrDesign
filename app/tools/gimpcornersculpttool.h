/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpcornersculpttool.h
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

#pragma once

#include "gimppathtool.h"

G_BEGIN_DECLS

#define GIMP_TYPE_CORNER_SCULPT_TOOL            (gimp_corner_sculpt_tool_get_type ())
#define GIMP_CORNER_SCULPT_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_CORNER_SCULPT_TOOL, GimpCornerSculptTool))
#define GIMP_CORNER_SCULPT_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_CORNER_SCULPT_TOOL, GimpCornerSculptToolClass))
#define GIMP_IS_CORNER_SCULPT_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_CORNER_SCULPT_TOOL))
#define GIMP_IS_CORNER_SCULPT_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_CORNER_SCULPT_TOOL))
#define GIMP_CORNER_SCULPT_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GIMP_TYPE_CORNER_SCULPT_TOOL, GimpCornerSculptToolClass))


typedef struct _GimpCornerSculptTool      GimpCornerSculptTool;
typedef struct _GimpCornerSculptToolClass GimpCornerSculptToolClass;

struct _GimpCornerSculptTool
{
  GimpPathTool parent_instance;

  gulong       mode_notify_id;
  gulong       radius_notify_id;
};

struct _GimpCornerSculptToolClass
{
  GimpPathToolClass parent_class;
};

GType gimp_corner_sculpt_tool_get_type (void) G_GNUC_CONST;

void  gimp_corner_sculpt_tool_register (GimpToolRegisterCallback callback,
                                        gpointer                 data);

G_END_DECLS
