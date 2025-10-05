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

#include <gegl.h>
#include <gtk/gtk.h>

#include "core/gimp.h"
#include "core/gimpcontext.h"
#include "core/gimpimage.h"
#include "core/gimpimage-undo.h"
#include "core/gimpundostack.h"
#include "core/gimplayer.h"

#include "gimp-app-test-utils.h"

#include "path/gimpbezierstroke.h"
#include "path/gimppathboolean.h"
#include "path/gimpvectorlayer.h"

#include "tests.h"


typedef struct
{
  GimpImage   *image;
  GimpContext *context;
} ShapeFusionFixture;


#define ADD_IMAGE_TEST(function) \
  g_test_add ("/gimp-shape-fusion/" #function, \
              ShapeFusionFixture, \
              gimp, \
              shape_fusion_setup, \
              function, \
              shape_fusion_teardown)


static void shape_fusion_setup    (ShapeFusionFixture *fixture,
                                   gconstpointer       data);
static void shape_fusion_teardown (ShapeFusionFixture *fixture,
                                   gconstpointer       data);

static GimpVectorLayer * shape_fusion_create_layer (ShapeFusionFixture *fixture,
                                                    const gchar        *name,
                                                    gdouble             offset);

static void test_shape_fusion_union       (ShapeFusionFixture *fixture,
                                           gconstpointer       data);
static void test_shape_fusion_subtract    (ShapeFusionFixture *fixture,
                                           gconstpointer       data);
static void test_shape_fusion_intersect   (ShapeFusionFixture *fixture,
                                           gconstpointer       data);
static void test_shape_fusion_preview     (ShapeFusionFixture *fixture,
                                           gconstpointer       data);


static void
shape_fusion_setup (ShapeFusionFixture *fixture,
                    gconstpointer       data)
{
  Gimp *gimp = GIMP (data);

  fixture->image = gimp_image_new (gimp,
                                   256,
                                   256,
                                   GIMP_RGB,
                                   GIMP_PRECISION_FLOAT_LINEAR);
  fixture->context = gimp_context_new (gimp, "Shape Fusion", NULL);
}

static void
shape_fusion_teardown (ShapeFusionFixture *fixture,
                       gconstpointer       data)
{
  g_clear_object (&fixture->context);
  g_clear_object (&fixture->image);
}

static GimpVectorLayer *
shape_fusion_create_layer (ShapeFusionFixture *fixture,
                           const gchar        *name,
                           gdouble             offset)
{
  GimpPath   *path;
  GimpStroke *stroke;
  GimpCoords  p1 = { offset,         offset,         0.0, 0.0 };
  GimpCoords  p2 = { offset + 64.0, offset,         0.0, 0.0 };
  GimpCoords  p3 = { offset + 64.0, offset + 64.0, 0.0, 0.0 };
  GimpCoords  p4 = { offset,         offset + 64.0, 0.0, 0.0 };
  GimpVectorLayer *layer;

  path = gimp_path_new (fixture->image, name);
  stroke = gimp_bezier_stroke_new_moveto (&p1);
  gimp_bezier_stroke_lineto (stroke, &p2);
  gimp_bezier_stroke_lineto (stroke, &p3);
  gimp_bezier_stroke_lineto (stroke, &p4);
  gimp_stroke_close (stroke);
  gimp_path_stroke_add (path, stroke);

  layer = gimp_vector_layer_new (fixture->image, path, fixture->context);
  gimp_object_set_name (G_OBJECT (layer), name);

  gimp_image_add_layer (fixture->image, GIMP_LAYER (layer), NULL, -1, FALSE);

  g_object_unref (path);

  return layer;
}

static void
shape_fusion_assert_preview (ShapeFusionFixture *fixture,
                             GimpVectorLayer    *target,
                             GList              *sources,
                             GimpPathBooleanMode mode,
                             gint                expected_strokes)
{
  GimpPath *preview = gimp_path_boolean_preview (target, sources, mode);

  g_assert_nonnull (preview);
  g_assert_cmpint (gimp_path_get_n_strokes (preview), ==, expected_strokes);

  g_object_unref (preview);
}

static void
shape_fusion_assert_apply (ShapeFusionFixture *fixture,
                           GimpVectorLayer    *target,
                           GList              *sources,
                           GimpPathBooleanMode mode,
                           gint                expected_strokes)
{
  GError *error = NULL;
  gboolean applied;
  GimpPath *path;

  applied = gimp_path_boolean_apply (fixture->image,
                                     target,
                                     sources,
                                     mode,
                                     TRUE,
                                     &error);

  g_assert_no_error (error);
  g_assert_true (applied);

  path = gimp_vector_layer_get_path (target);
  g_assert_cmpint (gimp_path_get_n_strokes (path), ==, expected_strokes);
}

static void
shape_fusion_check_undo_depth (ShapeFusionFixture *fixture)
{
  GimpUndoStack *stack = gimp_image_get_undo_stack (fixture->image);

  g_assert_cmpint (gimp_undo_stack_get_depth (stack), >, 0);
}

static void
test_shape_fusion_union (ShapeFusionFixture *fixture,
                          gconstpointer       data)
{
  GimpVectorLayer *base    = shape_fusion_create_layer (fixture, "base", 16.0);
  GimpVectorLayer *overlay = shape_fusion_create_layer (fixture, "overlay", 48.0);
  GList           *sources = NULL;

  sources = g_list_append (sources, overlay);

  shape_fusion_assert_preview (fixture, base, sources,
                               GIMP_PATH_BOOLEAN_MODE_UNION, 2);
  shape_fusion_assert_apply (fixture, base, sources,
                             GIMP_PATH_BOOLEAN_MODE_UNION, 2);

  shape_fusion_check_undo_depth (fixture);

  g_list_free (sources);
}

static void
test_shape_fusion_subtract (ShapeFusionFixture *fixture,
                             gconstpointer       data)
{
  GimpVectorLayer *base    = shape_fusion_create_layer (fixture, "base", 16.0);
  GimpVectorLayer *overlay = shape_fusion_create_layer (fixture, "overlay", 48.0);
  GList           *sources = NULL;

  sources = g_list_append (sources, overlay);

  shape_fusion_assert_preview (fixture, base, sources,
                               GIMP_PATH_BOOLEAN_MODE_SUBTRACT, 0);
  shape_fusion_assert_apply (fixture, base, sources,
                             GIMP_PATH_BOOLEAN_MODE_SUBTRACT, 0);

  shape_fusion_check_undo_depth (fixture);

  g_list_free (sources);
}

static void
test_shape_fusion_intersect (ShapeFusionFixture *fixture,
                              gconstpointer       data)
{
  GimpVectorLayer *base    = shape_fusion_create_layer (fixture, "base", 16.0);
  GimpVectorLayer *overlay = shape_fusion_create_layer (fixture, "overlay", 48.0);
  GList           *sources = NULL;

  sources = g_list_append (sources, overlay);

  shape_fusion_assert_preview (fixture, base, sources,
                               GIMP_PATH_BOOLEAN_MODE_INTERSECTION, 1);
  shape_fusion_assert_apply (fixture, base, sources,
                             GIMP_PATH_BOOLEAN_MODE_INTERSECTION, 1);

  shape_fusion_check_undo_depth (fixture);

  g_list_free (sources);
}

static void
test_shape_fusion_preview (ShapeFusionFixture *fixture,
                            gconstpointer       data)
{
  GimpVectorLayer *base = shape_fusion_create_layer (fixture, "base", 16.0);

  shape_fusion_assert_preview (fixture, base, NULL,
                               GIMP_PATH_BOOLEAN_MODE_UNION, 1);
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

  ADD_IMAGE_TEST (test_shape_fusion_union);
  ADD_IMAGE_TEST (test_shape_fusion_subtract);
  ADD_IMAGE_TEST (test_shape_fusion_intersect);
  ADD_IMAGE_TEST (test_shape_fusion_preview);

  result = g_test_run ();

  gimp_test_utils_set_gimp3_directory ("GIMP_TESTING_ABS_TOP_BUILDDIR",
                                       "app/tests/gimpdir-output");

  gimp_exit (gimp, TRUE);

  return result;
}
