/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * intelliselect-backend.h
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

#include "libgimpbase/gimpbase.h"
#include "core/gimpimage.h"
#include "core/gimpprogress.h"

G_BEGIN_DECLS

GeglBuffer * gimp_intelliselect_backend_run (GimpImage            *image,
                                             const gchar          *model_id,
                                             const gchar          *backend_id,
                                             GeglBuffer           *stroke_buffer,
                                             const GeglRectangle  *stroke_bounds,
                                             GimpProgress         *progress);

G_END_DECLS
