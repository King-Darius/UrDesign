/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * intelliselect-backend.c
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

#include <gio/gio.h>
#include <gegl.h>

#include "libgimpbase/gimpbase.h"

#include "intelliselect-backend.h"

#include "libgimp/libgimp-intl.h"

#define DEFAULT_MODEL_PATH "intelliselect/models"

static gchar *
resolve_model_path (const gchar *model_id)
{
  const gchar *base = g_getenv ("GIMP_INTELLISELECT_MODEL_PATH");
  if (! base || !*base)
    base = DEFAULT_MODEL_PATH;

  if (! model_id || !*model_id)
    model_id = "default";

  return g_build_filename (base, model_id, NULL);
}

static void
simulate_inference (GeglBuffer          *stroke_buffer,
                    const GeglRectangle *stroke_bounds,
                    GeglBuffer          *mask,
                    GimpProgress        *progress)
{
  gegl_buffer_copy (stroke_buffer, NULL, mask, NULL);

  if (progress)
    gimp_progress_set_value (progress, 1.0);
}

GeglBuffer *
gimp_intelliselect_backend_run (GimpImage            *image,
                                 const gchar          *model_id,
                                 const gchar          *backend_id,
                                 GeglBuffer           *stroke_buffer,
                                 const GeglRectangle  *stroke_bounds,
                                 GimpProgress         *progress)
{
  GeglRectangle rect;
  GeglBuffer   *mask;
  gchar        *model_path;

  g_return_val_if_fail (image == NULL || GIMP_IS_IMAGE (image), NULL);
  g_return_val_if_fail (stroke_buffer != NULL, NULL);
  g_return_val_if_fail (stroke_bounds != NULL, NULL);

  rect = *stroke_bounds;

  mask = gegl_buffer_new (&rect, babl_format ("Y u8"));

  if (progress)
    {
      gchar *message = g_strdup_printf (_("Running %s on %s"),
                                        model_id ? model_id : "default",
                                        backend_id ? backend_id : "auto");
      gimp_progress_start (progress, message, FALSE);
      g_free (message);
      gimp_progress_set_value (progress, 0.05);
    }

  model_path = resolve_model_path (model_id);
  g_message ("Using IntelliSelect model at %s (backend=%s)",
             model_path,
             backend_id ? backend_id : "auto");
  g_free (model_path);

  simulate_inference (stroke_buffer, stroke_bounds, mask, progress);

  return mask;
}
