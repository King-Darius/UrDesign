/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpshapefusiontool.c
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
#include <glib.h>

#include "libgimpconfig/gimpconfig.h"

#include "tools-types.h"

#include "actions/procedure-commands.h"

#include "core/gimp.h"
#include "core/gimpchannel-select.h"
#include "core/gimpcontext.h"
#include "core/gimpimage.h"
#include "core/gimpimage-undo.h"
#include "core/gimpitem.h"
#include "core/gimpselection.h"
#include "core/gimptoolinfo.h"

#include "pdb/gimppdb.h"

#include "path/gimppath.h"

#include "widgets/gimphelp-ids.h"

#include "display/gimpdisplay.h"

#include "gimpshapefusionoptions.h"
#include "gimpshapefusiontool.h"
#include "gimptoolcontrol.h"

#include "gimp-intl.h"

static void     gimp_shape_fusion_tool_button_press (GimpTool              *tool,
                                                     const GimpCoords      *coords,
                                                     guint32                time,
                                                     GdkModifierType        state,
                                                     GimpButtonPressType    press_type,
                                                     GimpDisplay           *display);

static void     gimp_shape_fusion_tool_register_type (GType                 tool_type,
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

G_DEFINE_TYPE (GimpShapeFusionTool, gimp_shape_fusion_tool, GIMP_TYPE_TOOL)

static void
 gimp_shape_fusion_tool_class_init (GimpShapeFusionToolClass *klass)
{
  GimpToolClass *tool_class = GIMP_TOOL_CLASS (klass);

  tool_class->button_press = gimp_shape_fusion_tool_button_press;
}

static void
 gimp_shape_fusion_tool_init (GimpShapeFusionTool *tool)
{
  gimp_tool_control_set_tool_cursor (GIMP_TOOL (tool)->control,
                                     GIMP_TOOL_CURSOR_PATHS);
  gimp_tool_control_set_wants_click (GIMP_TOOL (tool)->control, TRUE);
  gimp_tool_control_set_wants_double_click (GIMP_TOOL (tool)->control, FALSE);
  gimp_tool_control_set_precision (GIMP_TOOL (tool)->control,
                                   GIMP_CURSOR_PRECISION_PIXEL_CENTER);
}

static GeglBuffer *
shape_fusion_duplicate_selection (GimpImage *image)
{
  GimpChannel *mask;
  GeglBuffer  *buffer = NULL;

  mask = gimp_image_get_mask (image);

  if (mask)
    {
      GeglBuffer *source = gimp_drawable_get_buffer (GIMP_DRAWABLE (mask));

      if (source)
        buffer = gegl_buffer_dup (source);
    }

  return buffer;
}

static void
shape_fusion_restore_selection (GimpImage   *image,
                                GeglBuffer  *saved)
{
  GimpChannel *mask;

  if (! saved)
    return;

  mask = gimp_image_get_mask (image);

  if (! mask)
    {
      g_object_unref (saved);
      return;
    }

  gimp_channel_select_buffer (mask,
                              C_("command", "Shape Fusion"),
                              saved,
                              0, 0,
                              GIMP_CHANNEL_OP_REPLACE,
                              FALSE,
                              0.0, 0.0);

  g_object_unref (saved);
}

static void
shape_fusion_clear_selection (GimpImage *image)
{
  GimpChannel *mask = gimp_image_get_mask (image);

  if (mask)
    gimp_channel_clear (mask, NULL, TRUE);
}

static void
shape_fusion_apply_selection (GimpShapeFusionOptions *options,
                              GList                  *paths,
                              GimpImage              *image)
{
  GimpChannelOps next_op = GIMP_CHANNEL_OP_REPLACE;
  GList         *iter;

  for (iter = paths; iter; iter = iter->next)
    {
      gimp_item_to_selection (GIMP_ITEM (iter->data),
                              next_op,
                              TRUE,
                              FALSE,
                              0.0,
                              0.0);

      switch (options->mode)
        {
        case GIMP_SHAPE_FUSION_MODE_UNION:
          next_op = GIMP_CHANNEL_OP_ADD;
          break;

        case GIMP_SHAPE_FUSION_MODE_SUBTRACT:
          next_op = GIMP_CHANNEL_OP_SUBTRACT;
          break;

        case GIMP_SHAPE_FUSION_MODE_INTERSECT:
          next_op = GIMP_CHANNEL_OP_INTERSECT;
          break;

        default:
          break;
        }
    }
}

static GimpPath *
shape_fusion_pick_result_path (GimpImage *image,
                               GHashTable *original_paths)
{
  GList    *selected = gimp_image_get_selected_paths (image);
  GList    *iter;
  GimpPath *result = NULL;

  for (iter = selected; iter; iter = iter->next)
    {
      if (! g_hash_table_contains (original_paths, iter->data))
        {
          result = iter->data;
          break;
        }
    }

  if (! result && selected)
    result = selected->data;

  return result;
}

static gboolean
shape_fusion_convert_selection_to_path (GimpImage   *image,
                                        GimpDisplay *display)
{
  GimpProcedure  *procedure;
  GimpValueArray *args;
  gboolean        success;

  procedure = gimp_pdb_lookup_procedure (image->gimp->pdb,
                                         "plug-in-sel2path");

  if (! procedure)
    return FALSE;

  args = gimp_procedure_get_arguments (procedure);

  g_value_set_enum (gimp_value_array_index (args, 0),
                    GIMP_RUN_NONINTERACTIVE);
  g_value_set_object (gimp_value_array_index (args, 1),
                      image);

  success = procedure_commands_run_procedure (procedure,
                                              image->gimp,
                                              GIMP_PROGRESS (display),
                                              args);

  gimp_value_array_unref (args);

  return success;
}

static void
shape_fusion_remove_sources (GimpImage *image,
                              GList     *paths)
{
  GList *iter;

  for (iter = paths; iter; iter = iter->next)
    {
      gimp_image_remove_path (image, iter->data, TRUE, NULL);
    }
}

static void
shape_fusion_register_result (GimpImage *image,
                              GimpPath  *result)
{
  if (! result)
    return;

  gimp_viewable_set_name (GIMP_VIEWABLE (result),
                          C_("paths", "Fusion Result"));
}

static void
 gimp_shape_fusion_tool_button_press (GimpTool              *tool,
                                      const GimpCoords      *coords,
                                      guint32                time,
                                      GdkModifierType        state,
                                      GimpButtonPressType    press_type,
                                      GimpDisplay           *display)
{
  GimpShapeFusionOptions *options;
  GimpImage              *image;
  GList                  *paths;
  GList                  *iter;
  GHashTable             *original_paths;
  GeglBuffer             *saved_selection = NULL;
  gboolean                success = FALSE;

  if (press_type != GIMP_BUTTON_PRESS_NORMAL)
    return;

  image = gimp_display_get_image (display);
  if (! image)
    return;

  options = GIMP_SHAPE_FUSION_OPTIONS (tool->options);

  paths = g_list_copy (gimp_image_get_selected_paths (image));

  if (g_list_length (paths) < 2)
    {
      gimp_tool_message (tool, display,
                         _("Select at least two paths to fuse."));
      g_list_free (paths);
      return;
    }

  original_paths = g_hash_table_new (g_direct_hash, g_direct_equal);

  for (iter = paths; iter; iter = iter->next)
    g_hash_table_add (original_paths, iter->data);

  saved_selection = shape_fusion_duplicate_selection (image);

  gimp_image_undo_group_start (image, GIMP_UNDO_GROUP_PATHS,
                               C_("undo-type", "Shape Fusion"));

  shape_fusion_clear_selection (image);
  shape_fusion_apply_selection (options, paths, image);

  success = shape_fusion_convert_selection_to_path (image, display);

  if (success)
    {
      GimpPath *result = shape_fusion_pick_result_path (image, original_paths);

      if (! options->keep_sources)
        shape_fusion_remove_sources (image, paths);

      shape_fusion_register_result (image, result);
    }
  else
    {
      gimp_tool_message (tool, display,
                         _("Shape fusion failed. Ensure the selection plug-in is available."));
    }

  shape_fusion_restore_selection (image, saved_selection);

  gimp_image_flush (image);
  gimp_image_undo_group_end (image);

  g_hash_table_destroy (original_paths);
  g_list_free (paths);
}

static void
 gimp_shape_fusion_tool_register_type (GType                  tool_type,
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
 gimp_shape_fusion_tool_register (GimpToolRegisterCallback callback,
                                  gpointer                 data)
{
  gimp_shape_fusion_tool_register_type (GIMP_TYPE_SHAPE_FUSION_TOOL,
                                        GIMP_TYPE_SHAPE_FUSION_OPTIONS,
                                        gimp_shape_fusion_options_gui,
                                        0,
                                        "gimp-shape-fusion-tool",
                                        _("Shape Fusion"),
                                        _("Shape Fusion Tool"),
                                        N_("_Shape Fusion"), NULL,
                                        GIMP_HELP_TOOL_SHAPE_FUSION,
                                        GIMP_ICON_TOOL_PATH,
                                        callback);
}
