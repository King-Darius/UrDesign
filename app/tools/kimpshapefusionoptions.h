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

#include "core/gimptooloptions.h"

#include "path/gimppathboolean.h"

G_BEGIN_DECLS

#define KIMP_TYPE_SHAPE_FUSION_OPTIONS            (kimp_shape_fusion_options_get_type ())
#define KIMP_SHAPE_FUSION_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), KIMP_TYPE_SHAPE_FUSION_OPTIONS, KimpShapeFusionOptions))
#define KIMP_SHAPE_FUSION_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), KIMP_TYPE_SHAPE_FUSION_OPTIONS, KimpShapeFusionOptionsClass))
#define KIMP_IS_SHAPE_FUSION_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), KIMP_TYPE_SHAPE_FUSION_OPTIONS))
#define KIMP_IS_SHAPE_FUSION_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), KIMP_TYPE_SHAPE_FUSION_OPTIONS))
#define KIMP_SHAPE_FUSION_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), KIMP_TYPE_SHAPE_FUSION_OPTIONS, KimpShapeFusionOptionsClass))


typedef struct _KimpShapeFusionOptions      KimpShapeFusionOptions;
typedef struct _GimpToolOptionsClass        KimpShapeFusionOptionsClass;

struct _KimpShapeFusionOptions
{
  GimpToolOptions    parent_instance;

  GimpPathBooleanMode mode;
  gboolean            enable_additive_gesture;
  gboolean            enable_subtractive_gesture;
};


GType      kimp_shape_fusion_options_get_type (void) G_GNUC_CONST;

GtkWidget *kimp_shape_fusion_options_gui      (GimpToolOptions *tool_options);

G_END_DECLS
