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

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "libgimpbase/gimpbase.h"

#include "tools-types.h"

#include "core/gimp.h"
#include "core/gimpcontext.h"
#include "core/gimpimage.h"

#include "display/gimpdisplay.h"

#include "path/gimppathboolean.h"
#include "path/gimpvectorlayer.h"

#include "gimpshapefusiontool.h"

#include "gimp-intl.h"


static void     gimp_shape_fusion_tool_finalize      (GObject          *object);
static void     gimp_shape_fusion_tool_button_press  (GimpTool         *tool,
                                                      const GimpCoords *coords,
                                                      guint32           time,
                                                      GdkModifierType   state,
                                                      GimpButtonPressType press_type,
                                                      GimpDisplay      *display);
static void     gimp_shape_fusion_tool_button_release(GimpTool         *tool,
                                                      const GimpCoords *coords,
                                                      guint32           time,
                                                      GdkModifierType   state,
                                                      GimpButtonReleaseType release_type,
                                                      GimpDisplay      *display);
static void     gimp_shape_fusion_tool_motion        (GimpTool         *tool,
                                                      const GimpCoords *coords,
                                                      guint32           time,
                                                      GdkModifierType   state,
                                                      GimpDisplay      *display);
static void     gimp_shape_fusion_tool_oper_update   (GimpTool         *tool,
                                                      const GimpCoords *coords,
                                                      GdkModifierType   state,
                                                      gboolean          proximity,
                                                      GimpDisplay      *display);

static gboolean gimp_shape_fusion_tool_collect_layers(GimpShapeFusionTool *fusion_tool,
                                                      GimpImage           *image);
static void     gimp_shape_fusion_tool_clear_state   (GimpShapeFusionTool *fusion_tool);
static void     gimp_shape_fusion_tool_update_preview(GimpShapeFusionTool *fusion_tool,
                                                      GimpDisplay         *display);
static GimpPathBooleanMode
                gimp_shape_fusion_tool_resolve_mode (GimpShapeFusionTool *fusion_tool,
                                                      GdkModifierType      state);


G_DEFINE_TYPE (GimpShapeFusionTool, gimp_shape_fusion_tool, GIMP_TYPE_DRAW_TOOL)


void
gimp_shape_fusion_tool_register (GimpToolRegisterCallback  callback,
                                 gpointer                  data)
{
  (* callback) (GIMP_TYPE_SHAPE_FUSION_TOOL,
                KIMP_TYPE_SHAPE_FUSION_OPTIONS,
                kimp_shape_fusion_options_gui,
                0,
                "gimp-shape-fusion-tool",
                C_("tool", "Shape Fusion"),
                _("Shape Fusion Tool: Combine vector layers non-destructively"),
                N_("_Shape Fusion"), NULL,
                NULL, GIMP_HELP_TOOL_PATH,
                GIMP_ICON_TOOL_PATH,
                data);
}

static void
gimp_shape_fusion_tool_class_init (GimpShapeFusionToolClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  GimpToolClass *tool_class   = GIMP_TOOL_CLASS (klass);

  object_class->finalize     = gimp_shape_fusion_tool_finalize;

  tool_class->button_press   = gimp_shape_fusion_tool_button_press;
  tool_class->button_release = gimp_shape_fusion_tool_button_release;
  tool_class->motion         = gimp_shape_fusion_tool_motion;
  tool_class->oper_update    = gimp_shape_fusion_tool_oper_update;
}

static void
gimp_shape_fusion_tool_init (GimpShapeFusionTool *fusion_tool)
{
  fusion_tool->target       = NULL;
  fusion_tool->sources      = NULL;
  fusion_tool->preview_path = NULL;
  fusion_tool->mode         = GIMP_PATH_BOOLEAN_MODE_UNION;
}

static void
gimp_shape_fusion_tool_finalize (GObject *object)
{
  GimpShapeFusionTool *fusion_tool = GIMP_SHAPE_FUSION_TOOL (object);

  gimp_shape_fusion_tool_clear_state (fusion_tool);

  G_OBJECT_CLASS (gimp_shape_fusion_tool_parent_class)->finalize (object);
}

static void
gimp_shape_fusion_tool_button_press (GimpTool              *tool,
                                     const GimpCoords      *coords,
                                     guint32                time,
                                     GdkModifierType        state,
                                     GimpButtonPressType    press_type,
                                     GimpDisplay           *display)
{
  GimpShapeFusionTool     *fusion_tool = GIMP_SHAPE_FUSION_TOOL (tool);
  GimpImage               *image;
  KimpShapeFusionOptions  *options;

  gimp_shape_fusion_tool_clear_state (fusion_tool);

  image = gimp_display_get_image (display);
  if (! image)
    return;

  if (! gimp_shape_fusion_tool_collect_layers (fusion_tool, image))
    {
      gimp_tool_push_status (tool, display,
                             _("Select vector layers to preview Shape Fusion"));
      return;
    }

  options = KIMP_SHAPE_FUSION_OPTIONS (gimp_tool_get_options (tool));
  fusion_tool->mode = gimp_shape_fusion_tool_resolve_mode (fusion_tool, state);

  gimp_draw_tool_start (GIMP_DRAW_TOOL (tool), display);

  gimp_shape_fusion_tool_update_preview (fusion_tool, display);

  if (options->enable_additive_gesture || options->enable_subtractive_gesture)
    {
      GString *hint = g_string_new (NULL);

      if (options->enable_additive_gesture)
        g_string_append (hint, _("Hold Shift to union"));

      if (options->enable_subtractive_gesture)
        {
          if (hint->len > 0)
            g_string_append_c (hint, '\n');

          g_string_append (hint, _("Hold Alt to subtract"));
        }

      if (hint->len > 0)
        gimp_tool_push_status (tool, display, "%s", hint->str);

      g_string_free (hint, TRUE);
    }
}

static void
gimp_shape_fusion_tool_button_release (GimpTool               *tool,
                                       const GimpCoords       *coords,
                                       guint32                 time,
                                       GdkModifierType         state,
                                       GimpButtonReleaseType   release_type,
                                       GimpDisplay            *display)
{
  GimpShapeFusionTool *fusion_tool = GIMP_SHAPE_FUSION_TOOL (tool);
  GimpImage           *image       = gimp_display_get_image (display);

  if (fusion_tool->target && image)
    {
      GError *error = NULL;

      if (! gimp_path_boolean_apply (image,
                                     fusion_tool->target,
                                     fusion_tool->sources,
                                     fusion_tool->mode,
                                     TRUE,
                                     &error))
        {
          if (error)
            {
              gimp_tool_push_status (tool, display, "%s", error->message);
              g_clear_error (&error);
            }
        }
      else
        {
          gimp_tool_push_status (tool, display,
                                 _("Shape Fusion applied"));
        }
    }

  gimp_draw_tool_stop (GIMP_DRAW_TOOL (tool));
  gimp_shape_fusion_tool_clear_state (fusion_tool);
}

static void
gimp_shape_fusion_tool_motion (GimpTool         *tool,
                               const GimpCoords *coords,
                               guint32           time,
                               GdkModifierType   state,
                               GimpDisplay      *display)
{
  GimpShapeFusionTool    *fusion_tool = GIMP_SHAPE_FUSION_TOOL (tool);
  GimpPathBooleanMode     new_mode;

  if (! fusion_tool->target)
    return;

  new_mode = gimp_shape_fusion_tool_resolve_mode (fusion_tool, state);

  if (new_mode != fusion_tool->mode)
    {
      fusion_tool->mode = new_mode;
      gimp_shape_fusion_tool_update_preview (fusion_tool, display);
    }
}

static void
gimp_shape_fusion_tool_oper_update (GimpTool         *tool,
                                    const GimpCoords *coords,
                                    GdkModifierType   state,
                                    gboolean          proximity,
                                    GimpDisplay      *display)
{
  if (proximity)
    gimp_tool_push_status (tool, display,
                           _("Drag across vector layers to fuse shapes"));
}

static gboolean
gimp_shape_fusion_tool_collect_layers (GimpShapeFusionTool *fusion_tool,
                                       GimpImage           *image)
{
  GList           *selected;
  GList           *iter;
  GimpVectorLayer *target = NULL;
  GList           *sources = NULL;

  selected = gimp_image_get_selected_layers (image);

  for (iter = selected; iter; iter = iter->next)
    {
      if (! GIMP_IS_VECTOR_LAYER (iter->data))
        continue;

      if (! target)
        target = GIMP_VECTOR_LAYER (iter->data);
      else
        sources = g_list_append (sources, iter->data);
    }

  g_list_free (selected);

  if (! target)
    {
      GList *layers = gimp_image_get_layer_list (image);

      for (iter = layers; iter; iter = iter->next)
        {
          if (! GIMP_IS_VECTOR_LAYER (iter->data))
            continue;

          if (! target)
            target = GIMP_VECTOR_LAYER (iter->data);
          else
            sources = g_list_append (sources, iter->data);
        }

      g_list_free (layers);
    }

  if (! target)
    {
      g_list_free (sources);
      return FALSE;
    }

  fusion_tool->target  = target;
  fusion_tool->sources = sources;

  return TRUE;
}

static void
gimp_shape_fusion_tool_clear_state (GimpShapeFusionTool *fusion_tool)
{
  if (fusion_tool->sources)
    {
      g_list_free (fusion_tool->sources);
      fusion_tool->sources = NULL;
    }

  g_clear_object (&fusion_tool->preview_path);

  fusion_tool->target = NULL;
}

static void
gimp_shape_fusion_tool_update_preview (GimpShapeFusionTool *fusion_tool,
                                       GimpDisplay         *display)
{
  gint strokes = 0;

  g_clear_object (&fusion_tool->preview_path);

  if (! fusion_tool->target)
    return;

  fusion_tool->preview_path = gimp_path_boolean_preview (fusion_tool->target,
                                                         fusion_tool->sources,
                                                         fusion_tool->mode);

  if (fusion_tool->preview_path)
    strokes = gimp_path_get_n_strokes (fusion_tool->preview_path);

  gimp_tool_push_status (GIMP_TOOL (fusion_tool), display,
                         ngettext ("Preview contains %d stroke",
                                   "Preview contains %d strokes",
                                   strokes),
                         strokes);
}

static GimpPathBooleanMode
gimp_shape_fusion_tool_resolve_mode (GimpShapeFusionTool *fusion_tool,
                                     GdkModifierType      state)
{
  KimpShapeFusionOptions *options =
    KIMP_SHAPE_FUSION_OPTIONS (gimp_tool_get_options (GIMP_TOOL (fusion_tool)));

  if ((state & GDK_SHIFT_MASK) && options->enable_additive_gesture)
    return GIMP_PATH_BOOLEAN_MODE_UNION;

  if ((state & GDK_MOD1_MASK) && options->enable_subtractive_gesture)
    return GIMP_PATH_BOOLEAN_MODE_SUBTRACT;

  return options->mode;
}
