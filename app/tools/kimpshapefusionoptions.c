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
#include "libgimpconfig/gimpconfig.h"
#include "libgimpwidgets/gimpwidgets.h"

#include "tools-types.h"

#include "kimpshapefusionoptions.h"
#include "gimptooloptions-gui.h"

#include "gimp-intl.h"


enum
{
  PROP_0,
  PROP_BOOLEAN_MODE,
  PROP_ADDITIVE_GESTURE,
  PROP_SUBTRACTIVE_GESTURE
};


static void kimp_shape_fusion_options_set_property (GObject      *object,
                                                    guint         property_id,
                                                    const GValue *value,
                                                    GParamSpec   *pspec);
static void kimp_shape_fusion_options_get_property (GObject      *object,
                                                    guint         property_id,
                                                    GValue       *value,
                                                    GParamSpec   *pspec);


G_DEFINE_TYPE (KimpShapeFusionOptions, kimp_shape_fusion_options, GIMP_TYPE_TOOL_OPTIONS)


static void
kimp_shape_fusion_options_class_init (KimpShapeFusionOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = kimp_shape_fusion_options_set_property;
  object_class->get_property = kimp_shape_fusion_options_get_property;

  GIMP_CONFIG_PROP_ENUM (object_class, PROP_BOOLEAN_MODE,
                         "mode",
                         NULL, NULL,
                         GIMP_TYPE_PATH_BOOLEAN_MODE,
                         GIMP_PATH_BOOLEAN_MODE_UNION,
                         GIMP_PARAM_STATIC_STRINGS);

  GIMP_CONFIG_PROP_BOOLEAN (object_class, PROP_ADDITIVE_GESTURE,
                            "enable-additive-gesture",
                            NULL, NULL,
                            TRUE,
                            GIMP_PARAM_STATIC_STRINGS);

  GIMP_CONFIG_PROP_BOOLEAN (object_class, PROP_SUBTRACTIVE_GESTURE,
                            "enable-subtractive-gesture",
                            NULL, NULL,
                            TRUE,
                            GIMP_PARAM_STATIC_STRINGS);
}

static void
kimp_shape_fusion_options_init (KimpShapeFusionOptions *options)
{
  options->mode                      = GIMP_PATH_BOOLEAN_MODE_UNION;
  options->enable_additive_gesture   = TRUE;
  options->enable_subtractive_gesture = TRUE;
}

static void
kimp_shape_fusion_options_set_property (GObject      *object,
                                        guint         property_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  KimpShapeFusionOptions *options = KIMP_SHAPE_FUSION_OPTIONS (object);

  switch (property_id)
    {
    case PROP_BOOLEAN_MODE:
      options->mode = g_value_get_enum (value);
      break;

    case PROP_ADDITIVE_GESTURE:
      options->enable_additive_gesture = g_value_get_boolean (value);
      break;

    case PROP_SUBTRACTIVE_GESTURE:
      options->enable_subtractive_gesture = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
kimp_shape_fusion_options_get_property (GObject    *object,
                                        guint       property_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
  KimpShapeFusionOptions *options = KIMP_SHAPE_FUSION_OPTIONS (object);

  switch (property_id)
    {
    case PROP_BOOLEAN_MODE:
      g_value_set_enum (value, options->mode);
      break;

    case PROP_ADDITIVE_GESTURE:
      g_value_set_boolean (value, options->enable_additive_gesture);
      break;

    case PROP_SUBTRACTIVE_GESTURE:
      g_value_set_boolean (value, options->enable_subtractive_gesture);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
kimp_shape_fusion_options_gui (GimpToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox   = gimp_tool_options_gui (tool_options);
  GtkWidget *combo;
  GtkWidget *button;

  combo = gimp_prop_enum_combo_box_new (config, "mode", 0, 0);
  gimp_int_combo_box_set_label (GIMP_INT_COMBO_BOX (combo),
                                _("Fusion mode"));
  gtk_box_pack_start (GTK_BOX (vbox), combo, FALSE, FALSE, 0);

  button = gimp_prop_check_button_new (config,
                                       "enable-additive-gesture",
                                       _("Shift toggles union"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  button = gimp_prop_check_button_new (config,
                                       "enable-subtractive-gesture",
                                       _("Alt toggles subtract"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  return vbox;
}
