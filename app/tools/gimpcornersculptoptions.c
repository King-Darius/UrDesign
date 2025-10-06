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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libgimpconfig/gimpconfig.h"
#include "libgimpwidgets/gimpwidgets.h"

#include "tools-types.h"

#include "widgets/gimppropwidgets.h"

#include "gimpcornersculptoptions.h"
#include "gimptooloptions-gui.h"

#include "gimp-intl.h"

enum
{
  PROP_0,
  PROP_RADIUS,
  PROP_MODE
};

static void   gimp_corner_sculpt_options_set_property (GObject      *object,
                                                       guint         property_id,
                                                       const GValue *value,
                                                       GParamSpec   *pspec);
static void   gimp_corner_sculpt_options_get_property (GObject      *object,
                                                       guint         property_id,
                                                       GValue       *value,
                                                       GParamSpec   *pspec);

G_DEFINE_TYPE (GimpCornerSculptOptions, gimp_corner_sculpt_options,
               GIMP_TYPE_TOOL_OPTIONS)

static void
gimp_corner_sculpt_options_class_init (GimpCornerSculptOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = gimp_corner_sculpt_options_set_property;
  object_class->get_property = gimp_corner_sculpt_options_get_property;

  GIMP_CONFIG_PROP_DOUBLE (object_class, PROP_RADIUS,
                           "corner-radius",
                           _("Radius"),
                           _("Corner radius"),
                           0.0, 512.0, 12.0,
                           GIMP_PARAM_STATIC_STRINGS);

  GIMP_CONFIG_PROP_ENUM (object_class, PROP_MODE,
                         "corner-mode",
                         _("Corner Style"),
                         _("Corner profile"),
                         GIMP_TYPE_CORNER_SCULPT_MODE,
                         GIMP_CORNER_SCULPT_MODE_ROUND,
                         GIMP_PARAM_STATIC_STRINGS);
}

static void
gimp_corner_sculpt_options_init (GimpCornerSculptOptions *options)
{
  options->radius = 12.0;
  options->mode   = GIMP_CORNER_SCULPT_MODE_ROUND;
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
    case PROP_RADIUS:
      options->radius = g_value_get_double (value);
      break;

    case PROP_MODE:
      options->mode = g_value_get_enum (value);
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
    case PROP_RADIUS:
      g_value_set_double (value, options->radius);
      break;

    case PROP_MODE:
      g_value_set_enum (value, options->mode);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
gimp_corner_sculpt_options_gui (GimpToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox;
  GtkWidget *widget;

  vbox = gimp_tool_options_gui_new (tool_options);

  widget = gimp_prop_spin_scale_new (config, "corner-radius",
                                     1.0, 64.0, 1);
  gimp_scale_entry_set_digits (GIMP_SCALE_ENTRY (widget), 2);
  gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);

  widget = gimp_prop_enum_combo_box_new (config, "corner-mode", 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);

  return vbox;
}
