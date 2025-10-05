/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * test-intelliselect-backend.c
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

#include "plug-ins/intelliselect/intelliselect-backend.h"

static void
set_point (GeglBuffer *buffer,
           gint        x,
           gint        y)
{
  guchar        value = 255;
  GeglRectangle rect  = { x, y, 1, 1 };

  gegl_buffer_set (buffer, &rect, 0, babl_format ("Y u8"), &value, GEGL_AUTO_ROWSTRIDE);
}

static guchar
get_point (GeglBuffer *buffer,
           gint        x,
           gint        y)
{
  guchar        value = 0;
  GeglRectangle rect  = { x, y, 1, 1 };

  gegl_buffer_get (buffer, &rect, 0, babl_format ("Y u8"), &value,
                   GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

  return value;
}

static void
backend_copies_points (void)
{
  GeglRectangle bounds = { 0, 0, 8, 8 };
  GeglBuffer   *stroke = gegl_buffer_new (&bounds, babl_format ("Y u8"));
  GeglBuffer   *mask;

  set_point (stroke, 2, 3);

  mask = gimp_intelliselect_backend_run (NULL,
                                         "stub-model",
                                         "cpu",
                                         stroke,
                                         &bounds,
                                         NULL);

  g_assert_nonnull (mask);
  g_assert_cmpint (get_point (mask, 2, 3), ==, 255);

  g_object_unref (mask);
  g_object_unref (stroke);
}

static void
selection_buffer_updates (void)
{
  GeglRectangle bounds = { 0, 0, 4, 4 };
  GeglBuffer   *stroke = gegl_buffer_new (&bounds, babl_format ("Y u8"));
  GeglBuffer   *mask;
  GeglBuffer   *selection = gegl_buffer_new (&bounds, babl_format ("Y u8"));

  set_point (stroke, 1, 1);
  set_point (stroke, 2, 2);

  mask = gimp_intelliselect_backend_run (NULL,
                                         "stub-model",
                                         "cpu",
                                         stroke,
                                         &bounds,
                                         NULL);

  g_assert_nonnull (mask);

  gegl_buffer_copy (mask, NULL, selection, NULL);

  g_assert_cmpint (get_point (selection, 2, 2), ==, 255);

  g_object_unref (selection);
  g_object_unref (mask);
  g_object_unref (stroke);
}

int
main (int    argc,
      char **argv)
{
  gegl_init (&argc, &argv);
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/plug-ins/intelliselect/backend-copies", backend_copies_points);
  g_test_add_func ("/plug-ins/intelliselect/selection-buffer-updates", selection_buffer_updates);

  return g_test_run ();
}
