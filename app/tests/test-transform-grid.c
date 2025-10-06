/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 2024 The GIMP Team
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

#include "libgimpmath/gimpmatrix.h"

#include "display/gimpdisplay.h"
#include "display/gimpdisplayshell.h"
#include "display/gimptooltransformgrid.h"
#include "display/gimptoolwidget.h"

#include "widgets/gimpwidgets-utils.h"

#include "core/gimp.h"
#include "core/gimpimage.h"

#include "gimp-app-test-utils.h"
#include "tests.h"

#define GRID_WIDTH   64
#define GRID_HEIGHT  64

static GimpDisplay *
get_only_display (Gimp *gimp)
{
  GList *iter;

  g_assert_true (g_list_length (gimp_get_image_iter (gimp)) == 1);

  iter = gimp_get_display_iter (gimp);
  g_assert_true (g_list_length (iter) == 1);

  return GIMP_DISPLAY (iter->data);
}

static GimpImage *
get_only_image (Gimp *gimp)
{
  GList *iter = gimp_get_image_iter (gimp);

  g_assert_true (g_list_length (iter) == 1);

  return GIMP_IMAGE (iter->data);
}

static void
run_modifier_cycle (Gimp     *gimp,
                    gboolean  baseline)
{
  GimpDisplay      *display;
  GimpDisplayShell *shell;
  GimpImage        *image;
  GimpToolWidget   *widget;
  GimpMatrix3       transform;
  GdkModifierType   extend_mask;
  gboolean          cornersnap;
  gboolean          constrain_move;
  gboolean          constrain_scale;
  gboolean          constrain_rotate;
  gboolean          constrain_shear;
  gboolean          constrain_perspective;

  gimp_test_utils_create_image (gimp, GRID_WIDTH, GRID_HEIGHT);
  gimp_test_run_mainloop_until_idle ();

  display = get_only_display (gimp);
  shell   = gimp_display_get_shell (display);
  image   = get_only_image (gimp);

  gimp_matrix3_identity (&transform);
  widget = gimp_tool_transform_grid_new (shell,
                                         &transform,
                                         0.0, 0.0,
                                         GRID_WIDTH,
                                         GRID_HEIGHT);

  g_object_set (widget,
                "cornersnap",            baseline,
                "constrain-move",        baseline,
                "constrain-scale",       baseline,
                "constrain-rotate",      baseline,
                "constrain-shear",       baseline,
                "constrain-perspective", baseline,
                NULL);
  g_object_get (widget,
                "cornersnap",            &cornersnap,
                "constrain-move",        &constrain_move,
                "constrain-scale",       &constrain_scale,
                "constrain-rotate",      &constrain_rotate,
                "constrain-shear",       &constrain_shear,
                "constrain-perspective", &constrain_perspective,
                NULL);
  g_assert_cmpint (cornersnap, ==, baseline);
  g_assert_cmpint (constrain_move, ==, baseline);
  g_assert_cmpint (constrain_scale, ==, baseline);
  g_assert_cmpint (constrain_rotate, ==, baseline);
  g_assert_cmpint (constrain_shear, ==, baseline);
  g_assert_cmpint (constrain_perspective, ==, baseline);

  extend_mask = gimp_get_extend_selection_mask ();

  gimp_tool_widget_hover_modifier (widget, extend_mask, TRUE, extend_mask);
  g_object_get (widget,
                "cornersnap",            &cornersnap,
                "constrain-move",        &constrain_move,
                "constrain-scale",       &constrain_scale,
                "constrain-rotate",      &constrain_rotate,
                "constrain-shear",       &constrain_shear,
                "constrain-perspective", &constrain_perspective,
                NULL);
  g_assert_cmpint (cornersnap, ==, ! baseline);
  g_assert_cmpint (constrain_move, ==, ! baseline);
  g_assert_true (constrain_scale);
  g_assert_cmpint (constrain_rotate, ==, ! baseline);
  g_assert_cmpint (constrain_shear, ==, ! baseline);
  g_assert_cmpint (constrain_perspective, ==, ! baseline);

  gimp_tool_widget_hover_modifier (widget, extend_mask, FALSE, 0);
  g_object_get (widget,
                "cornersnap",            &cornersnap,
                "constrain-move",        &constrain_move,
                "constrain-scale",       &constrain_scale,
                "constrain-rotate",      &constrain_rotate,
                "constrain-shear",       &constrain_shear,
                "constrain-perspective", &constrain_perspective,
                NULL);
  g_assert_cmpint (cornersnap, ==, baseline);
  g_assert_cmpint (constrain_move, ==, baseline);
  g_assert_cmpint (constrain_scale, ==, baseline);
  g_assert_cmpint (constrain_rotate, ==, baseline);
  g_assert_cmpint (constrain_shear, ==, baseline);
  g_assert_cmpint (constrain_perspective, ==, baseline);

  /* Verify that a subsequent cycle leaves the baseline intact. */
  gimp_tool_widget_hover_modifier (widget, extend_mask, TRUE, extend_mask);
  g_object_get (widget,
                "cornersnap",            &cornersnap,
                "constrain-move",        &constrain_move,
                "constrain-scale",       &constrain_scale,
                "constrain-rotate",      &constrain_rotate,
                "constrain-shear",       &constrain_shear,
                "constrain-perspective", &constrain_perspective,
                NULL);
  g_assert_cmpint (cornersnap, ==, ! baseline);
  g_assert_cmpint (constrain_move, ==, ! baseline);
  g_assert_true (constrain_scale);
  g_assert_cmpint (constrain_rotate, ==, ! baseline);
  g_assert_cmpint (constrain_shear, ==, ! baseline);
  g_assert_cmpint (constrain_perspective, ==, ! baseline);

  gimp_tool_widget_hover_modifier (widget, extend_mask, FALSE, 0);
  g_object_get (widget,
                "cornersnap",            &cornersnap,
                "constrain-move",        &constrain_move,
                "constrain-scale",       &constrain_scale,
                "constrain-rotate",      &constrain_rotate,
                "constrain-shear",       &constrain_shear,
                "constrain-perspective", &constrain_perspective,
                NULL);
  g_assert_cmpint (cornersnap, ==, baseline);
  g_assert_cmpint (constrain_move, ==, baseline);
  g_assert_cmpint (constrain_scale, ==, baseline);
  g_assert_cmpint (constrain_rotate, ==, baseline);
  g_assert_cmpint (constrain_shear, ==, baseline);
  g_assert_cmpint (constrain_perspective, ==, baseline);

  g_object_unref (widget);

  gimp_display_close (display);
  g_object_unref (image);
  gimp_test_run_mainloop_until_idle ();
}

static void
modifier_respects_default_off (gconstpointer data)
{
  run_modifier_cycle (GIMP (data), FALSE);
}

static void
modifier_respects_default_on (gconstpointer data)
{
  run_modifier_cycle (GIMP (data), TRUE);
}

int
main (int    argc,
      char **argv)
{
  Gimp *gimp;
  int   result;

  g_test_init (&argc, &argv, NULL);

  gimp_test_utils_set_gimp3_directory ("GIMP_TESTING_ABS_TOP_SRCDIR",
                                       "app/tests/gimpdir");

  gimp = gimp_init_for_testing ();

  g_test_add_data_func ("/gimp-transform-grid/extend-modifier-respects-default-off",
                        gimp,
                        modifier_respects_default_off);
  g_test_add_data_func ("/gimp-transform-grid/extend-modifier-respects-default-on",
                        gimp,
                        modifier_respects_default_on);

  result = g_test_run ();

  gimp_test_utils_set_gimp3_directory ("GIMP_TESTING_ABS_TOP_BUILDDIR",
                                       "app/tests/gimpdir-output");

  gimp_exit (gimp, TRUE);

  return result;
}
