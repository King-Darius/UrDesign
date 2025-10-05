/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpcornersculptoptions.c
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

#include "libgimpbase/gimpbase.h"

#include "tools-types.h"

#include "core/gimp.h"

#include "widgets/gimppropwidgets.h"

#include "gimpcornersculptoptions.h"

#include "gimp-intl.h"


enum
{
  PROP_0,
  PROP_CORNER_MODE,
  PROP_CORNER_RADIUS
};

static void      gimp_corner_sculpt_options_set_property (GObject      *object,
                                                          guint         property_id,
                                                          const GValue *value,
                                                          GParamSpec   *pspec);
static void      gimp_corner_sculpt_options_get_property (GObject      *object,
                                                          guint         property_id,
                                                          GValue       *value,
                                                          GParamSpec   *pspec);
static GtkWidget *gimp_corner_sculpt_options_create_popover (GimpCornerSculptOptions *options,
                                                             GtkWidget               *button);


G_DEFINE_TYPE (GimpCornerSculptOptions, gimp_corner_sculpt_options,
               GIMP_TYPE_PATH_OPTIONS)

GType
gimp_corner_mode_get_type (void)
{
  static GType type_id = 0;

  if (g_once_init_enter (&type_id))
    {
      static const GEnumValue values[] =
      {
        { GIMP_CORNER_MODE_CHAMFER,  "GIMP_CORNER_MODE_CHAMFER",  "chamfer"  },
        { GIMP_CORNER_MODE_ROUND,    "GIMP_CORNER_MODE_ROUND",    "round"    },
        { GIMP_CORNER_MODE_INVERTED, "GIMP_CORNER_MODE_INVERTED", "inverted" },
        { 0, NULL, NULL }
      };
      GType tmp = g_enum_register_static ("GimpCornerMode", values);
      g_once_init_leave (&type_id, tmp);
    }

  return type_id;
}

static void
gimp_corner_sculpt_options_class_init (GimpCornerSculptOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = gimp_corner_sculpt_options_set_property;
  object_class->get_property = gimp_corner_sculpt_options_get_property;

  g_object_class_install_property (object_class, PROP_CORNER_MODE,
                                   g_param_spec_enum ("corner-mode",
                                                      "Corner Mode",
                                                      "Corner shape applied to selected nodes",
                                                      GIMP_TYPE_CORNER_MODE,
                                                      GIMP_CORNER_MODE_CHAMFER,
                                                      GIMP_PARAM_STATIC_STRINGS |
                                                      G_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_CORNER_RADIUS,
                                   g_param_spec_double ("corner-radius",
                                                        "Corner Radius",
                                                        "Corner radius to apply during sculpting",
                                                        0.0, 1000.0, 0.0,
                                                        GIMP_PARAM_STATIC_STRINGS |
                                                        G_PARAM_READWRITE));
}

static void
gimp_corner_sculpt_options_init (GimpCornerSculptOptions *options)
{
  options->corner_mode   = GIMP_CORNER_MODE_CHAMFER;
  options->corner_radius = 0.0;
}

static void
gimp_corner_sculpt_options_set_property (GObject      *object,
                                         guint         property_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
  GimpCornerSculptOptions *options = GIMP_CORNER_SCULPT_OPTIONS (object);

  switch (property_id)
    {
    case PROP_CORNER_MODE:
      options->corner_mode = g_value_get_enum (value);
      break;

    case PROP_CORNER_RADIUS:
      options->corner_radius = g_value_get_double (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gimp_corner_sculpt_options_get_property (GObject    *object,
                                         guint       property_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
  GimpCornerSculptOptions *options = GIMP_CORNER_SCULPT_OPTIONS (object);

  switch (property_id)
    {
    case PROP_CORNER_MODE:
      g_value_set_enum (value, options->corner_mode);
      break;

    case PROP_CORNER_RADIUS:
      g_value_set_double (value, options->corner_radius);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
gimp_corner_sculpt_options_gui (GimpToolOptions *tool_options)
{
  GtkWidget               *vbox;
  GtkWidget               *button;
  GimpCornerSculptOptions *options = GIMP_CORNER_SCULPT_OPTIONS (tool_options);

  vbox = gimp_path_options_gui (tool_options);

  button = gtk_menu_button_new ();
  gtk_button_set_label (GTK_BUTTON (button), _("Corner Controls"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  gtk_menu_button_set_popover (GTK_MENU_BUTTON (button),
                               gimp_corner_sculpt_options_create_popover (options,
                                                                           button));

  return vbox;
}

static GtkWidget *
gimp_corner_sculpt_options_create_popover (GimpCornerSculptOptions *options,
                                             GtkWidget               *button)
{
  GtkWidget *popover;
  GtkWidget *grid;
  GtkWidget *mode_box;
  GtkWidget *radius;

  popover = gtk_popover_new (button);

  grid = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_widget_set_margin_start (grid, 12);
  gtk_widget_set_margin_end (grid, 12);
  gtk_widget_set_margin_top (grid, 12);
  gtk_widget_set_margin_bottom (grid, 12);
  gtk_widget_show (grid);
  gtk_container_add (GTK_CONTAINER (popover), grid);

  mode_box = gimp_prop_enum_radio_box_new (G_OBJECT (options),
                                           "corner-mode", 1, 0);
  gtk_box_pack_start (GTK_BOX (grid), mode_box, FALSE, FALSE, 0);
  gtk_widget_show (mode_box);

  radius = gimp_prop_spin_button_new (G_OBJECT (options), "corner-radius",
                                      0.1, 1.0, 2);
  gtk_box_pack_start (GTK_BOX (grid), radius, FALSE, FALSE, 0);
  gtk_widget_show (radius);

  return popover;
}
