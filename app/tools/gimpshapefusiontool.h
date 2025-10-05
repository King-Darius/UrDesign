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

#pragma once

#include "gimpdrawtool.h"

#include "kimpshapefusionoptions.h"

G_BEGIN_DECLS

#define GIMP_TYPE_SHAPE_FUSION_TOOL            (gimp_shape_fusion_tool_get_type ())
#define GIMP_SHAPE_FUSION_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_SHAPE_FUSION_TOOL, GimpShapeFusionTool))
#define GIMP_SHAPE_FUSION_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_SHAPE_FUSION_TOOL, GimpShapeFusionToolClass))
#define GIMP_IS_SHAPE_FUSION_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_SHAPE_FUSION_TOOL))
#define GIMP_IS_SHAPE_FUSION_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_SHAPE_FUSION_TOOL))
#define GIMP_SHAPE_FUSION_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GIMP_TYPE_SHAPE_FUSION_TOOL, GimpShapeFusionToolClass))


typedef struct _GimpShapeFusionTool       GimpShapeFusionTool;
typedef struct _GimpShapeFusionToolClass  GimpShapeFusionToolClass;

struct _GimpShapeFusionTool
{
  GimpDrawTool          parent_instance;

  GimpVectorLayer      *target;
  GList                *sources;
  GimpPathBooleanMode   mode;
  GimpPath             *preview_path;
};

struct _GimpShapeFusionToolClass
{
  GimpDrawToolClass parent_class;
};


GType      gimp_shape_fusion_tool_get_type (void) G_GNUC_CONST;

void       gimp_shape_fusion_tool_register (GimpToolRegisterCallback  callback,
                                            gpointer                  data);

G_END_DECLS
