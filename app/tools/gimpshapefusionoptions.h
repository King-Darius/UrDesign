/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpshapefusionoptions.h
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

#include "gimptooloptions.h"

#define GIMP_TYPE_SHAPE_FUSION_OPTIONS            (gimp_shape_fusion_options_get_type ())
#define GIMP_SHAPE_FUSION_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_SHAPE_FUSION_OPTIONS, GimpShapeFusionOptions))
#define GIMP_SHAPE_FUSION_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_SHAPE_FUSION_OPTIONS, GimpShapeFusionOptionsClass))
#define GIMP_IS_SHAPE_FUSION_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_SHAPE_FUSION_OPTIONS))
#define GIMP_IS_SHAPE_FUSION_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_SHAPE_FUSION_OPTIONS))
#define GIMP_SHAPE_FUSION_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GIMP_TYPE_SHAPE_FUSION_OPTIONS, GimpShapeFusionOptionsClass))

typedef struct _GimpShapeFusionOptions      GimpShapeFusionOptions;
typedef struct _GimpShapeFusionOptionsClass GimpShapeFusionOptionsClass;

struct _GimpShapeFusionOptions
{
  GimpToolOptions     parent_instance;

  GimpShapeFusionMode mode;
  gboolean            keep_sources;
};

struct _GimpShapeFusionOptionsClass
{
  GimpToolOptionsClass parent_class;
};

GType      gimp_shape_fusion_options_get_type (void) G_GNUC_CONST;

GtkWidget *gimp_shape_fusion_options_gui      (GimpToolOptions *tool_options);
