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

#include <gtk/gtk.h>

#include "libgimpconfig/gimpconfig.h"
#include "libgimpwidgets/gimpwidgets.h"

#include "tools-types.h"

#include "widgets/gimppropwidgets.h"

#include "gimpintelliselectoptions.h"
#include "gimptooloptions-gui.h"

#include "gimp-intl.h"

enum
{
  PROP_0,
  PROP_RADIUS,
  PROP_THRESHOLD,
  PROP_SAMPLE_MERGED,
  PROP_REFINE_EDGES
};

static void   gimp_intelli_select_options_set_property (GObject      *object,
                                                        guint         property_id,
                                                        const GValue *value,
                                                        GParamSpec   *pspec);
static void   gimp_intelli_select_options_get_property (GObject      *object,
                                                        guint         property_id,
                                                        GValue       *value,
                                                        GParamSpec   *pspec);

G_DEFINE_TYPE (GimpIntelliSelectOptions, gimp_intelli_select_options,
               GIMP_TYPE_SELECTION_OPTIONS)

static void
 gimp_intelli_select_options_class_init (GimpIntelliSelectOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = gimp_intelli_select_options_set_property;
  object_class->get_property = gimp_intelli_select_options_get_property;

  GIMP_CONFIG_PROP_DOUBLE (object_class, PROP_RADIUS,
                           "sample-radius",
                           _("Sample radius"),
                           _("Radius for generating the smart selection context"),
                           1.0, 512.0, 32.0,
                           GIMP_PARAM_STATIC_STRINGS);

  GIMP_CONFIG_PROP_DOUBLE (object_class, PROP_THRESHOLD,
                           "detection-threshold",
                           _("Detection threshold"),
                           _("Normalized similarity threshold for segmentation"),
                           0.0, 1.0, 0.18,
                           GIMP_PARAM_STATIC_STRINGS);

  GIMP_CONFIG_PROP_BOOLEAN (object_class, PROP_SAMPLE_MERGED,
                            "sample-merged",
                            _("Sample merged"),
                            _("Use the composite of visible layers for sampling"),
                            TRUE,
                            GIMP_PARAM_STATIC_STRINGS);

  GIMP_CONFIG_PROP_BOOLEAN (object_class, PROP_REFINE_EDGES,
                            "refine-edges",
                            _("Refine edges"),
                            _("Smooth and expand the resulting selection mask"),
                            TRUE,
                            GIMP_PARAM_STATIC_STRINGS);
}

static void
 gimp_intelli_select_options_init (GimpIntelliSelectOptions *options)
{
  options->radius        = 32.0;
  options->threshold     = 0.18;
  options->sample_merged = TRUE;
  options->refine_edges  = TRUE;
}

static void
 gimp_intelli_select_options_set_property (GObject      *object,
                                           guint         property_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
  GimpIntelliSelectOptions *options = GIMP_INTELLI_SELECT_OPTIONS (object);

  switch (property_id)
    {
    case PROP_RADIUS:
      options->radius = g_value_get_double (value);
      break;

    case PROP_THRESHOLD:
      options->threshold = g_value_get_double (value);
      break;

    case PROP_SAMPLE_MERGED:
      options->sample_merged = g_value_get_boolean (value);
      break;

    case PROP_REFINE_EDGES:
      options->refine_edges = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
 gimp_intelli_select_options_get_property (GObject    *object,
                                           guint       property_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
  GimpIntelliSelectOptions *options = GIMP_INTELLI_SELECT_OPTIONS (object);

  switch (property_id)
    {
    case PROP_RADIUS:
      g_value_set_double (value, options->radius);
      break;

    case PROP_THRESHOLD:
      g_value_set_double (value, options->threshold);
      break;

    case PROP_SAMPLE_MERGED:
      g_value_set_boolean (value, options->sample_merged);
      break;

    case PROP_REFINE_EDGES:
      g_value_set_boolean (value, options->refine_edges);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
 gimp_intelli_select_options_gui (GimpToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox;
  GtkWidget *widget;

  vbox = gimp_tool_options_gui_new (tool_options);

  widget = gimp_prop_spin_scale_new (config, "sample-radius",
                                     1.0, 256.0, 1);
  gimp_scale_entry_set_digits (GIMP_SCALE_ENTRY (widget), 1);
  gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);

  widget = gimp_prop_spin_scale_new (config, "detection-threshold",
                                     0.01, 1.0, 0.01);
  gimp_scale_entry_set_digits (GIMP_SCALE_ENTRY (widget), 2);
  gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);

  widget = gimp_prop_check_button_new (config,
                                       "sample-merged",
                                       _("Sample merged"));
  gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);

  widget = gimp_prop_check_button_new (config,
                                       "refine-edges",
                                       _("Refine edges"));
  gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);

  return vbox;
}
