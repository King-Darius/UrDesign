/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpintelliselecttool.h
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

#include "gimpselectiontool.h"

#define GIMP_TYPE_INTELLI_SELECT_TOOL            (gimp_intelli_select_tool_get_type ())
#define GIMP_INTELLI_SELECT_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_INTELLI_SELECT_TOOL, GimpIntelliSelectTool))
#define GIMP_INTELLI_SELECT_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_INTELLI_SELECT_TOOL, GimpIntelliSelectToolClass))
#define GIMP_IS_INTELLI_SELECT_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_INTELLI_SELECT_TOOL))
#define GIMP_IS_INTELLI_SELECT_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_INTELLI_SELECT_TOOL))
#define GIMP_INTELLI_SELECT_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GIMP_TYPE_INTELLI_SELECT_TOOL, GimpIntelliSelectToolClass))

typedef struct _GimpIntelliSelectTool      GimpIntelliSelectTool;
typedef struct _GimpIntelliSelectToolClass GimpIntelliSelectToolClass;

struct _GimpIntelliSelectTool
{
  GimpSelectionTool parent_instance;
};

struct _GimpIntelliSelectToolClass
{
  GimpSelectionToolClass parent_class;
};

GType gimp_intelli_select_tool_get_type (void) G_GNUC_CONST;

void  gimp_intelli_select_tool_register (GimpToolRegisterCallback callback,
                                         gpointer                 data);
