/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpintelliselecttool.c
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

#include <gegl.h>

#include "libgimpconfig/gimpconfig.h"

#include "tools-types.h"

#include "actions/procedure-commands.h"

#include "core/gimp.h"
#include "core/gimpcontext.h"
#include "core/gimpdrawable.h"
#include "core/gimpimage.h"
#include "core/gimptoolinfo.h"

#include "pdb/gimppdb.h"

#include "display/gimpdisplay.h"

#include "widgets/gimphelp-ids.h"

#include "gimpintelliselectoptions.h"
#include "gimpintelliselecttool.h"
#include "gimptoolcontrol.h"

#include "gimp-intl.h"

static void     gimp_intelli_select_tool_button_press (GimpTool              *tool,
                                                       const GimpCoords      *coords,
                                                       guint32                time,
                                                       GdkModifierType        state,
                                                       GimpButtonPressType    press_type,
                                                       GimpDisplay           *display);

static void     gimp_intelli_select_tool_register_type (GType                 tool_type,
                                                        GType                 options_type,
                                                        GimpToolOptionsGUIFunc gui_func,
                                                        GimpContextPropMask   context_props,
                                                        const gchar          *identifier,
                                                        const gchar          *label,
                                                        const gchar          *tooltip,
                                                        const gchar          *menu_label,
                                                        const gchar          *menu_accel,
                                                        const gchar          *help_domain,
                                                        const gchar          *help_id,
                                                        const gchar          *icon_name,
                                                        gpointer              data);

G_DEFINE_TYPE (GimpIntelliSelectTool, gimp_intelli_select_tool,
               GIMP_TYPE_SELECTION_TOOL)

static void
 gimp_intelli_select_tool_class_init (GimpIntelliSelectToolClass *klass)
{
  GimpToolClass *tool_class = GIMP_TOOL_CLASS (klass);

  tool_class->button_press = gimp_intelli_select_tool_button_press;
}

static void
 gimp_intelli_select_tool_init (GimpIntelliSelectTool *tool)
{
  gimp_tool_control_set_tool_cursor (GIMP_TOOL (tool)->control,
                                     GIMP_TOOL_CURSOR_FUZZY);
  gimp_tool_control_set_wants_click (GIMP_TOOL (tool)->control, TRUE);
  gimp_tool_control_set_wants_double_click (GIMP_TOOL (tool)->control, FALSE);
  gimp_tool_control_set_precision (GIMP_TOOL (tool)->control,
                                   GIMP_CURSOR_PRECISION_PIXEL_CENTER);
}

static gboolean
intelli_select_invoke_procedure (GimpImage              *image,
                                 GimpDrawable          *drawable,
                                 GimpIntelliSelectOptions *options,
                                 const GimpCoords      *coords,
                                 GimpDisplay           *display)
{
  GimpProcedure  *procedure;
  GimpValueArray *args;
  gboolean        success;

  procedure = gimp_pdb_lookup_procedure (image->gimp->pdb,
                                         "plug-in-intelli-select");

  if (! procedure)
    return FALSE;

  args = gimp_procedure_get_arguments (procedure);

  g_value_set_enum (gimp_value_array_index (args, 0),
                    GIMP_RUN_NONINTERACTIVE);
  g_value_set_object (gimp_value_array_index (args, 1),
                      image);
  g_value_set_object (gimp_value_array_index (args, 2),
                      drawable);
  g_value_set_double (gimp_value_array_index (args, 3), coords->x);
  g_value_set_double (gimp_value_array_index (args, 4), coords->y);
  g_value_set_double (gimp_value_array_index (args, 5), options->radius);
  g_value_set_double (gimp_value_array_index (args, 6), options->threshold);
  g_value_set_boolean (gimp_value_array_index (args, 7), options->sample_merged);
  g_value_set_boolean (gimp_value_array_index (args, 8), options->refine_edges);

  success = procedure_commands_run_procedure (procedure,
                                              image->gimp,
                                              GIMP_PROGRESS (display),
                                              args);

  gimp_value_array_unref (args);

  return success;
}

static void
 gimp_intelli_select_tool_button_press (GimpTool              *tool,
                                        const GimpCoords      *coords,
                                        guint32                time,
                                        GdkModifierType        state,
                                        GimpButtonPressType    press_type,
                                        GimpDisplay           *display)
{
  GimpIntelliSelectOptions *options;
  GimpImage                *image;
  GimpDrawable             *drawable;
  gboolean                  success;

  if (press_type != GIMP_BUTTON_PRESS_NORMAL)
    return;

  image = gimp_display_get_image (display);
  if (! image)
    return;

  drawable = gimp_image_get_active_drawable (image);

  if (! drawable)
    {
      gimp_tool_message (tool, display,
                         _("No active drawable to analyze."));
      return;
    }

  options = GIMP_INTELLI_SELECT_OPTIONS (tool->options);

  success = intelli_select_invoke_procedure (image,
                                             drawable,
                                             options,
                                             coords,
                                             display);

  if (! success)
    {
      gimp_tool_message (tool, display,
                         _("IntelliSelect plug-in is unavailable."));
      return;
    }

  gimp_image_flush (image);
}

static void
 gimp_intelli_select_tool_register_type (GType                  tool_type,
                                         GType                  options_type,
                                         GimpToolOptionsGUIFunc options_gui_func,
                                         GimpContextPropMask    context_props,
                                         const gchar           *identifier,
                                         const gchar           *label,
                                         const gchar           *tooltip,
                                         const gchar           *menu_label,
                                         const gchar           *menu_accel,
                                         const gchar           *help_domain,
                                         const gchar           *help_id,
                                         const gchar           *icon_name,
                                         gpointer               data)
{
  GimpToolRegisterCallback callback = (GimpToolRegisterCallback) data;

  callback (tool_type,
            options_type,
            options_gui_func,
            context_props,
            identifier,
            label,
            tooltip,
            menu_label,
            menu_accel,
            help_domain,
            help_id,
            icon_name,
            NULL);
}

void
 gimp_intelli_select_tool_register (GimpToolRegisterCallback callback,
                                    gpointer                 data)
{
  gimp_intelli_select_tool_register_type (GIMP_TYPE_INTELLI_SELECT_TOOL,
                                          GIMP_TYPE_INTELLI_SELECT_OPTIONS,
                                          gimp_intelli_select_options_gui,
                                          0,
                                          "gimp-intelli-select-tool",
                                          _("Intelli Select"),
                                          _("Intelli Select Tool"),
                                          N_("_Intelli Select"), NULL,
                                          GIMP_HELP_TOOL_INTELLI_SELECT,
                                          GIMP_ICON_TOOL_PAINT_SELECT,
                                          callback);
}
