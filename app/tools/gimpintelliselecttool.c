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
#include <math.h>

#include "libgimpbase/gimpbase.h"

#include "tools-types.h"

#include "config/gimpguiconfig.h"

#include "core/gimp.h"
#include "core/gimpchannel-select.h"
#include "core/gimpdrawable.h"
#include "core/gimpimage.h"
#include "core/gimpprogress.h"

#include "display/gimpdisplay.h"

#include "gimpintelliselectoptions.h"
#include "gimpintelliselecttool.h"
#include "gimpselectiontool.h"
#include "gimptoolcontrol.h"
#include "gimptool-progress.h"

#include "gimp-intl.h"

#include "gegl/gimp-gegl-apply-operation.h"

#include "plug-ins/intelliselect/intelliselect-backend.h"

struct _GimpIntelliSelectTool
{
  GimpSelectionTool parent_instance;

  GArray       *stroke_points;
  GeglRectangle stroke_bounds;
  gboolean      has_bounds;
  gboolean      collecting;
};

G_DEFINE_TYPE (GimpIntelliSelectTool, gimp_intelli_select_tool,
               GIMP_TYPE_SELECTION_TOOL)

#define parent_class gimp_intelli_select_tool_parent_class

typedef struct
{
  gdouble x;
  gdouble y;
} StrokePoint;

static void     gimp_intelli_select_tool_finalize       (GObject             *object);
static void     gimp_intelli_select_tool_control        (GimpTool            *tool,
                                                         GimpToolAction       action,
                                                         GimpDisplay         *display);
static void     gimp_intelli_select_tool_button_press   (GimpTool            *tool,
                                                         const GimpCoords    *coords,
                                                         guint32              time,
                                                         GdkModifierType      state,
                                                         GimpButtonPressType  press_type,
                                                         GimpDisplay         *display);
static void     gimp_intelli_select_tool_button_release (GimpTool            *tool,
                                                         const GimpCoords    *coords,
                                                         guint32              time,
                                                         GdkModifierType      state,
                                                         GimpButtonReleaseType release_type,
                                                         GimpDisplay         *display);
static void     gimp_intelli_select_tool_motion         (GimpTool            *tool,
                                                         const GimpCoords    *coords,
                                                         guint32              time,
                                                         GdkModifierType      state,
                                                         GimpDisplay         *display);

static void     gimp_intelli_select_tool_reset_stream   (GimpIntelliSelectTool *tool);
static void     gimp_intelli_select_tool_append_point   (GimpIntelliSelectTool *tool,
                                                         const GimpCoords      *coords);
static void     gimp_intelli_select_tool_run_inference  (GimpIntelliSelectTool *tool,
                                                         GimpDisplay           *display);


void
gimp_intelli_select_tool_register (GimpToolRegisterCallback  callback,
                                   gpointer                  data)
{
  (* callback) (GIMP_TYPE_INTELLI_SELECT_TOOL,
                GIMP_TYPE_INTELLI_SELECT_OPTIONS,
                gimp_selection_options_gui,
                0,
                "gimp-intelli-select-tool",
                _("Intelligent Select"),
                _("Intelligent Select Tool: Segment regions using on-device inference"),
                N_("_Intelligent Select"), NULL,
                NULL, GIMP_HELP_TOOL_FOREGROUND_SELECT,
                GIMP_ICON_TOOL_FOREGROUND_SELECT,
                data);
}

static void
gimp_intelli_select_tool_class_init (GimpIntelliSelectToolClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GimpToolClass  *tool_class   = GIMP_TOOL_CLASS (klass);

  object_class->finalize = gimp_intelli_select_tool_finalize;

  tool_class->control        = gimp_intelli_select_tool_control;
  tool_class->button_press   = gimp_intelli_select_tool_button_press;
  tool_class->button_release = gimp_intelli_select_tool_button_release;
  tool_class->motion         = gimp_intelli_select_tool_motion;

  gimp_tool_class_set_tool_info (tool_class,
                                 _("Intelligent Select"),
                                 GIMP_ICON_TOOL_FOREGROUND_SELECT,
                                 GIMP_CURSOR_CROSSHAIR,
                                 GIMP_TOOL_CURSOR_SELECTION);
}

static void
gimp_intelli_select_tool_init (GimpIntelliSelectTool *tool)
{
  tool->stroke_points = g_array_new (FALSE, FALSE, sizeof (StrokePoint));
  tool->has_bounds    = FALSE;
  tool->collecting    = FALSE;

  gimp_tool_control_set_wants_clicks (GIMP_TOOL (tool)->control, TRUE);
  gimp_tool_control_set_scroll_lock  (GIMP_TOOL (tool)->control, TRUE);
}

static void
gimp_intelli_select_tool_finalize (GObject *object)
{
  GimpIntelliSelectTool *tool = GIMP_INTELLI_SELECT_TOOL (object);

  g_clear_pointer (&tool->stroke_points, g_array_unref);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gimp_intelli_select_tool_control (GimpTool       *tool,
                                  GimpToolAction  action,
                                  GimpDisplay    *display)
{
  GimpIntelliSelectTool *is_tool = GIMP_INTELLI_SELECT_TOOL (tool);

  if (action == GIMP_TOOL_ACTION_HALT)
    gimp_intelli_select_tool_reset_stream (is_tool);

  GIMP_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static void
gimp_intelli_select_tool_button_press (GimpTool            *tool,
                                       const GimpCoords    *coords,
                                       guint32              time,
                                       GdkModifierType      state,
                                       GimpButtonPressType  press_type,
                                       GimpDisplay         *display)
{
  GimpIntelliSelectTool *is_tool = GIMP_INTELLI_SELECT_TOOL (tool);

  gimp_intelli_select_tool_reset_stream (is_tool);
  gimp_intelli_select_tool_append_point (is_tool, coords);

  is_tool->collecting = TRUE;

  if (GIMP_TOOL_CLASS (parent_class)->button_press)
    GIMP_TOOL_CLASS (parent_class)->button_press (tool, coords, time, state, press_type, display);
}

static void
gimp_intelli_select_tool_button_release (GimpTool             *tool,
                                         const GimpCoords     *coords,
                                         guint32               time,
                                         GdkModifierType       state,
                                         GimpButtonReleaseType release_type,
                                         GimpDisplay          *display)
{
  GimpIntelliSelectTool *is_tool = GIMP_INTELLI_SELECT_TOOL (tool);

  if (is_tool->collecting)
    {
      gimp_intelli_select_tool_append_point (is_tool, coords);
      gimp_intelli_select_tool_run_inference (is_tool, display);
    }

  is_tool->collecting = FALSE;

  if (GIMP_TOOL_CLASS (parent_class)->button_release)
    GIMP_TOOL_CLASS (parent_class)->button_release (tool, coords, time, state, release_type, display);
}

static void
gimp_intelli_select_tool_motion (GimpTool           *tool,
                                 const GimpCoords   *coords,
                                 guint32             time,
                                 GdkModifierType     state,
                                 GimpDisplay        *display)
{
  GimpIntelliSelectTool *is_tool = GIMP_INTELLI_SELECT_TOOL (tool);

  if (is_tool->collecting)
    gimp_intelli_select_tool_append_point (is_tool, coords);

  if (GIMP_TOOL_CLASS (parent_class)->motion)
    GIMP_TOOL_CLASS (parent_class)->motion (tool, coords, time, state, display);
}

static void
update_bounds (GimpIntelliSelectTool *tool,
               const StrokePoint     *point)
{
  if (! tool->has_bounds)
    {
      tool->stroke_bounds.x      = floor (point->x);
      tool->stroke_bounds.y      = floor (point->y);
      tool->stroke_bounds.width  = 1;
      tool->stroke_bounds.height = 1;
      tool->has_bounds = TRUE;
      return;
    }

  gint x = floor (point->x);
  gint y = floor (point->y);

  gint x2 = MAX (tool->stroke_bounds.x + tool->stroke_bounds.width,  x + 1);
  gint y2 = MAX (tool->stroke_bounds.y + tool->stroke_bounds.height, y + 1);

  tool->stroke_bounds.x      = MIN (tool->stroke_bounds.x, x);
  tool->stroke_bounds.y      = MIN (tool->stroke_bounds.y, y);
  tool->stroke_bounds.width  = x2 - tool->stroke_bounds.x;
  tool->stroke_bounds.height = y2 - tool->stroke_bounds.y;
}

static void
gimp_intelli_select_tool_reset_stream (GimpIntelliSelectTool *tool)
{
  g_array_set_size (tool->stroke_points, 0);
  tool->has_bounds = FALSE;
}

static void
gimp_intelli_select_tool_append_point (GimpIntelliSelectTool *tool,
                                       const GimpCoords      *coords)
{
  StrokePoint point = { coords->x, coords->y };

  g_array_append_val (tool->stroke_points, point);
  update_bounds (tool, &point);
}

static GeglBuffer *
gimp_intelli_select_tool_create_scribble (GimpIntelliSelectTool *tool,
                                          GeglRectangle         *roi)
{
  if (! tool->has_bounds || tool->stroke_bounds.width <= 0 || tool->stroke_bounds.height <= 0)
    return NULL;

  GeglRectangle rect = tool->stroke_bounds;
  GeglBuffer   *buffer;

  buffer = gegl_buffer_new (&rect, babl_format ("Y u8"));

  for (guint i = 0; i < tool->stroke_points->len; i++)
    {
      StrokePoint point = g_array_index (tool->stroke_points, StrokePoint, i);
      GeglRectangle px = { floor (point.x), floor (point.y), 1, 1 };
      guchar value = 255;
      gegl_buffer_set (buffer, &px, 0, babl_format ("Y u8"), &value, GEGL_AUTO_ROWSTRIDE);
    }

  if (roi)
    *roi = rect;

  return buffer;
}

static void
apply_mask_to_selection (GimpImage   *image,
                         GeglBuffer  *mask,
                         gint         offset_x,
                         gint         offset_y,
                         const gchar *undo_desc)
{
  GimpChannel *selection = gimp_image_get_mask (image);

  gimp_channel_select_buffer (selection,
                              undo_desc,
                              mask,
                              offset_x,
                              offset_y,
                              GIMP_CHANNEL_OP_REPLACE,
                              0, 0,
                              FALSE,
                              0.0,
                              FALSE,
                              NULL);
}

static void
gimp_intelli_select_tool_run_inference (GimpIntelliSelectTool *tool,
                                        GimpDisplay           *display)
{
  GimpImage                  *image;
  GimpIntelliSelectOptions   *options;
  GeglBuffer                 *scribble;
  GeglRectangle               roi;
  GeglBuffer                 *mask = NULL;
  GimpProgress               *progress = NULL;
  GimpTool                   *gtool;

  g_return_if_fail (GIMP_IS_DISPLAY (display));

  image   = gimp_display_get_image (display);
  gtool   = GIMP_TOOL (tool);
  if (gtool->tool_info && gtool->tool_info->tool_options)
    options = GIMP_INTELLI_SELECT_OPTIONS (gtool->tool_info->tool_options);

  scribble = gimp_intelli_select_tool_create_scribble (tool, &roi);
  if (! scribble)
    return;

  progress = gimp_tool_progress_new (gtool, display);

  mask = gimp_intelliselect_backend_run (image,
                                         options ? gimp_intelli_select_options_get_model (options) : NULL,
                                         options ? gimp_intelli_select_options_get_backend (options) : NULL,
                                         scribble,
                                         &roi,
                                         progress);

  if (mask)
    {
      apply_mask_to_selection (image, mask, roi.x, roi.y,
                               C_("undo-type", "Intelligent Select"));
      g_object_unref (mask);
    }

  g_object_unref (scribble);

  if (progress)
    gimp_progress_end (progress);
}
