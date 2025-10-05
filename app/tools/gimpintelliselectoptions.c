/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpintelliselectoptions.c
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

#include "gimpintelliselectoptions.h"

#include "config/gimpguiconfig.h"

struct _GimpIntelliSelectOptions
{
  GimpSelectionOptions parent_instance;

  gchar   *model;
  gchar   *backend;
};

G_DEFINE_TYPE (GimpIntelliSelectOptions, gimp_intelli_select_options,
               GIMP_TYPE_SELECTION_OPTIONS)

enum
{
  PROP_0,
  PROP_MODEL,
  PROP_BACKEND,
  PROP_LAST
};

static GParamSpec *props[PROP_LAST];

static void
GimpIntelliSelectOptions_finalize (GObject *object)
{
  GimpIntelliSelectOptions *options = GIMP_INTELLI_SELECT_OPTIONS (object);

  g_clear_pointer (&options->model, g_free);
  g_clear_pointer (&options->backend, g_free);

  G_OBJECT_CLASS (gimp_intelli_select_options_parent_class)->finalize (object);
}

static void
GimpIntelliSelectOptions_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  GimpIntelliSelectOptions *options = GIMP_INTELLI_SELECT_OPTIONS (object);

  switch (property_id)
    {
    case PROP_MODEL:
      gimp_intelli_select_options_set_model (options, g_value_get_string (value));
      break;

    case PROP_BACKEND:
      gimp_intelli_select_options_set_backend (options, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
GimpIntelliSelectOptions_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  GimpIntelliSelectOptions *options = GIMP_INTELLI_SELECT_OPTIONS (object);

  switch (property_id)
    {
    case PROP_MODEL:
      g_value_set_string (value, options->model);
      break;

    case PROP_BACKEND:
      g_value_set_string (value, options->backend);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
GimpIntelliSelectOptions_class_init (GimpIntelliSelectOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize     = GimpIntelliSelectOptions_finalize;
  object_class->set_property = GimpIntelliSelectOptions_set_property;
  object_class->get_property = GimpIntelliSelectOptions_get_property;

  props[PROP_MODEL] =
    g_param_spec_string ("model",
                         "model",
                         "The identifier of the model used for inference",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  props[PROP_BACKEND] =
    g_param_spec_string ("backend",
                         "backend",
                         "The identifier of the inference backend",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, PROP_LAST, props);
}

static void
GimpIntelliSelectOptions_init (GimpIntelliSelectOptions *options)
{
  options->model   = g_strdup ("builtin-default");
  options->backend = g_strdup ("auto");
}

const gchar *
gimp_intelli_select_options_get_model (GimpIntelliSelectOptions *options)
{
  g_return_val_if_fail (GIMP_IS_INTELLI_SELECT_OPTIONS (options), NULL);

  return options->model;
}

const gchar *
gimp_intelli_select_options_get_backend (GimpIntelliSelectOptions *options)
{
  g_return_val_if_fail (GIMP_IS_INTELLI_SELECT_OPTIONS (options), NULL);

  return options->backend;
}

void
gimp_intelli_select_options_set_model (GimpIntelliSelectOptions *options,
                                         const gchar             *model)
{
  g_return_if_fail (GIMP_IS_INTELLI_SELECT_OPTIONS (options));

  if (g_strcmp0 (options->model, model) == 0)
    return;

  g_free (options->model);
  options->model = g_strdup (model);

  g_object_notify_by_pspec (G_OBJECT (options), props[PROP_MODEL]);
}

void
gimp_intelli_select_options_set_backend (GimpIntelliSelectOptions *options,
                                           const gchar             *backend)
{
  g_return_if_fail (GIMP_IS_INTELLI_SELECT_OPTIONS (options));

  if (g_strcmp0 (options->backend, backend) == 0)
    return;

  g_free (options->backend);
  options->backend = g_strdup (backend);

  g_object_notify_by_pspec (G_OBJECT (options), props[PROP_BACKEND]);
}
