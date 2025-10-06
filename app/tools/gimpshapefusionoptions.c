/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpshapefusionoptions.c
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

#include "libgimpconfig/gimpconfig.h"
#include "libgimpwidgets/gimpwidgets.h"

#include "tools-types.h"

#include "widgets/gimppropwidgets.h"

#include "gimpshapefusionoptions.h"
#include "gimptooloptions-gui.h"

#include "gimp-intl.h"

enum
{
  PROP_0,
  PROP_MODE,
  PROP_KEEP_SOURCES
};

static void   gimp_shape_fusion_options_set_property (GObject      *object,
                                                      guint         property_id,
                                                      const GValue *value,
                                                      GParamSpec   *pspec);
static void   gimp_shape_fusion_options_get_property (GObject      *object,
                                                      guint         property_id,
                                                      GValue       *value,
                                                      GParamSpec   *pspec);

G_DEFINE_TYPE (GimpShapeFusionOptions, gimp_shape_fusion_options,
               GIMP_TYPE_TOOL_OPTIONS)

static void
gimp_shape_fusion_options_class_init (GimpShapeFusionOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = gimp_shape_fusion_options_set_property;
  object_class->get_property = gimp_shape_fusion_options_get_property;

  GIMP_CONFIG_PROP_ENUM (object_class, PROP_MODE,
                         "fusion-mode",
                         _("Combine"),
                         _("Boolean operation"),
                         GIMP_TYPE_SHAPE_FUSION_MODE,
                         GIMP_SHAPE_FUSION_MODE_UNION,
                         GIMP_PARAM_STATIC_STRINGS);

  GIMP_CONFIG_PROP_BOOLEAN (object_class, PROP_KEEP_SOURCES,
                            "keep-sources",
                            _("Preserve originals"),
                            _("Keep source paths when combining"),
                            FALSE,
                            GIMP_PARAM_STATIC_STRINGS);
}

static void
gimp_shape_fusion_options_init (GimpShapeFusionOptions *options)
{
  options->mode         = GIMP_SHAPE_FUSION_MODE_UNION;
  options->keep_sources = FALSE;
}

static void
gimp_shape_fusion_options_set_property (GObject      *object,
                                         guint         property_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
  GimpShapeFusionOptions *options = GIMP_SHAPE_FUSION_OPTIONS (object);

  switch (property_id)
    {
    case PROP_MODE:
      options->mode = g_value_get_enum (value);
      break;

    case PROP_KEEP_SOURCES:
      options->keep_sources = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gimp_shape_fusion_options_get_property (GObject    *object,
                                         guint       property_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
  GimpShapeFusionOptions *options = GIMP_SHAPE_FUSION_OPTIONS (object);

  switch (property_id)
    {
    case PROP_MODE:
      g_value_set_enum (value, options->mode);
      break;

    case PROP_KEEP_SOURCES:
      g_value_set_boolean (value, options->keep_sources);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
gimp_shape_fusion_options_gui (GimpToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox;
  GtkWidget *widget;

  vbox = gimp_tool_options_gui_new (tool_options);

  widget = gimp_prop_enum_combo_box_new (config, "fusion-mode", 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);

  widget = gimp_prop_check_button_new (config,
                                       "keep-sources",
                                       _("Preserve originals"));
  gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);

  return vbox;
}
