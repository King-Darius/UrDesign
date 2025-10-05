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

#include "gimpintelliselectoptions.h"

#define GIMP_TYPE_INTELLI_SELECT_TOOL (gimp_intelli_select_tool_get_type ())
G_DECLARE_FINAL_TYPE (GimpIntelliSelectTool, gimp_intelli_select_tool,
                      GIMP, INTELLI_SELECT_TOOL, GimpSelectionTool)

void gimp_intelli_select_tool_register (GimpToolRegisterCallback  callback,
                                        gpointer                  data);
