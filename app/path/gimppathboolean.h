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

#include "core/gimpimage.h"

#include "gimppath.h"
#include "gimpvectorlayer.h"

G_BEGIN_DECLS

#define GIMP_TYPE_PATH_BOOLEAN_MODE (gimp_path_boolean_mode_get_type ())

typedef enum
{
  GIMP_PATH_BOOLEAN_MODE_UNION,
  GIMP_PATH_BOOLEAN_MODE_INTERSECTION,
  GIMP_PATH_BOOLEAN_MODE_SUBTRACT
} GimpPathBooleanMode;

GType       gimp_path_boolean_mode_get_type (void) G_GNUC_CONST;

GimpPath  * gimp_path_boolean_preview       (GimpVectorLayer    *target,
                                             GList              *sources,
                                             GimpPathBooleanMode mode);

gboolean    gimp_path_boolean_apply         (GimpImage          *image,
                                             GimpVectorLayer    *target,
                                             GList              *sources,
                                             GimpPathBooleanMode mode,
                                             gboolean            push_undo,
                                             GError            **error);

G_END_DECLS
