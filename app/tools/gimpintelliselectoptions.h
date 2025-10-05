/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpintelliselectoptions.h
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

#include "gimpselectionoptions.h"

#define GIMP_TYPE_INTELLI_SELECT_OPTIONS (gimp_intelli_select_options_get_type ())
G_DECLARE_FINAL_TYPE (GimpIntelliSelectOptions, gimp_intelli_select_options,
                      GIMP, INTELLI_SELECT_OPTIONS, GimpSelectionOptions)

const gchar * gimp_intelli_select_options_get_model   (GimpIntelliSelectOptions *options);
const gchar * gimp_intelli_select_options_get_backend (GimpIntelliSelectOptions *options);

void gimp_intelli_select_options_set_model   (GimpIntelliSelectOptions *options,
                                              const gchar             *model);
void gimp_intelli_select_options_set_backend (GimpIntelliSelectOptions *options,
                                              const gchar             *backend);
