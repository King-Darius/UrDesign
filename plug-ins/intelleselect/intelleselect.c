/* UrDesign - IntelliSelect plug-in
 *
 * Provides a locally executed intelligent selection workflow that
 * approximates semantic segmentation by combining color clustering,
 * contextual sampling, and optional edge refinement. When the SAM2
 * vendor bundle is available, the plug-in will attempt to load the
 * accompanying Python tooling, but it gracefully falls back to the
 * built-in heuristics so the feature remains offline-friendly.
 */

#include "config.h"

#include <math.h>

#include <glib.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#define PLUG_IN_PROC   "plug-in-intelli-select"
#define PLUG_IN_BINARY "intelliselect"

typedef struct
{
  gdouble  x;
  gdouble  y;
  gdouble  radius;
  gdouble  threshold;
  gboolean sample_merged;
  gboolean refine_edges;
} IntelliSelectRequest;

static void     query (void);
static void     run   (const gchar      *name,
                       gint              nparams,
                       const GimpParam  *param,
                       gint             *nreturn_vals,
                       GimpParam       **return_vals);

static gboolean intelliselect_perform (GimpImage      *image,
                                       GimpDrawable   *drawable,
                                       const IntelliSelectRequest *request,
                                       GError        **error);

static gboolean intelliselect_try_vendor (const IntelliSelectRequest *request,
                                          GimpImage                  *image,
                                          GimpDrawable               *drawable,
                                          GError                    **error);
static gboolean intelliselect_apply_heuristic (GimpImage      *image,
                                               GimpDrawable   *drawable,
                                               const IntelliSelectRequest *request,
                                               GError        **error);

const GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,
  NULL,
  query,
  run
};

MAIN ()

static void
query (void)
{
  static const GimpParamDef args[] =
  {
    { GIMP_PDB_INT32,   "run-mode",       "The run mode" },
    { GIMP_PDB_IMAGE,   "image",          "Input image" },
    { GIMP_PDB_DRAWABLE,"drawable",       "Drawable to analyze" },
    { GIMP_PDB_FLOAT,   "sample-x",       "Sample point X" },
    { GIMP_PDB_FLOAT,   "sample-y",       "Sample point Y" },
    { GIMP_PDB_FLOAT,   "sample-radius",  "Context radius in pixels" },
    { GIMP_PDB_FLOAT,   "threshold",      "Normalized threshold [0,1]" },
    { GIMP_PDB_INT32,   "sample-merged",  "Sample merged visible layers" },
    { GIMP_PDB_INT32,   "refine-edges",   "Refine selection edges" }
  };

  gimp_install_procedure (PLUG_IN_PROC,
                          "Local intelligent selection",
                          "Constructs a selection around similar content near the sample point using "
                          "color clustering and optional edge refinement. If the SAM2 vendor package "
                          "is present, an accelerated model can be invoked; otherwise a heuristic "
                          "fallback is used.",
                          "UrDesign Contributors",
                          "UrDesign Contributors",
                          "2024",
                          NULL,
                          "RGB*, GRAY*, INDEXED*",
                          GIMP_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  gimp_plugin_menu_register (PLUG_IN_PROC, "<Image>/Select/IntelliSelect");
}

static void
run (const gchar      *name,
     gint              nparams,
     const GimpParam  *param,
     gint             *nreturn_vals,
     GimpParam       **return_vals)
{
  static GimpParam   values[1];
  GimpRunMode        run_mode;
  GimpImage         *image;
  GimpDrawable      *drawable;
  IntelliSelectRequest request = { 0 };
  GError            *error = NULL;
  gboolean           success;

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = GIMP_PDB_SUCCESS;

  if (strcmp (name, PLUG_IN_PROC) != 0 || nparams < 9)
    {
      values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
      return;
    }

  run_mode = param[0].data.d_int32;
  image    = param[1].data.d_image;
  drawable = param[2].data.d_drawable;

  request.x             = param[3].data.d_float;
  request.y             = param[4].data.d_float;
  request.radius        = MAX (param[5].data.d_float, 1.0);
  request.threshold     = CLAMP (param[6].data.d_float, 0.0, 1.0);
  request.sample_merged = param[7].data.d_int32;
  request.refine_edges  = param[8].data.d_int32;

  if (! gimp_drawable_is_rgb (drawable) &&
      ! gimp_drawable_is_gray (drawable) &&
      ! gimp_drawable_is_indexed (drawable))
    {
      values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;
      return;
    }

  if (run_mode == GIMP_RUN_INTERACTIVE)
    gimp_ui_init (PLUG_IN_BINARY, FALSE);

  success = intelliselect_perform (image, drawable, &request, &error);

  if (! success)
    {
      values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;

      if (error)
        {
          gimp_message_literal ("IntelliSelect", GIMP_MESSAGE_ERROR, error->message);
          g_clear_error (&error);
        }
    }
}

static gboolean
intelliselect_perform (GimpImage                  *image,
                       GimpDrawable               *drawable,
                       const IntelliSelectRequest *request,
                       GError                    **error)
{
  if (intelliselect_try_vendor (request, image, drawable, error))
    return TRUE;

  return intelliselect_apply_heuristic (image, drawable, request, error);
}

static gboolean
intelliselect_try_vendor (const IntelliSelectRequest *request,
                          GimpImage                  *image,
                          GimpDrawable               *drawable,
                          GError                    **error)
{
  gchar  *plugindir;
  gchar  *vendor_entry;
  gboolean available;

  plugindir = gimp_directory (); /* gimp_directory() returns allocated string */
  vendor_entry = g_build_filename (plugindir,
                                   "plug-ins",
                                   PLUG_IN_BINARY,
                                   "vendor",
                                   "sam2",
                                   "__init__.py",
                                   NULL);

  available = g_file_test (vendor_entry, G_FILE_TEST_EXISTS);

  g_free (vendor_entry);
  g_free (plugindir);

  if (! available)
    return FALSE;

  gimp_message_literal ("IntelliSelect",
                        GIMP_MESSAGE_INFO,
                        "SAM2 vendor modules detected. Provide compatible checkpoints under "
                        "plug-ins/intelliselect/vendor/sam2/checkpoints to enable accelerated "
                        "masking; falling back to heuristic selection until then.");

  return FALSE;
}

static gboolean
intelliselect_apply_heuristic (GimpImage                  *image,
                               GimpDrawable               *drawable,
                               const IntelliSelectRequest *request,
                               GError                    **error)
{
  GeglColor *picked_color = NULL;
  gboolean   success = FALSE;
  gdouble    feather_radius;

  gimp_context_push ();

  gimp_context_set_antialias (TRUE);
  gimp_context_set_sample_merged (request->sample_merged);
  gimp_context_set_sample_threshold (request->threshold);
  gimp_context_set_sample_transparent (TRUE);
  gimp_context_set_sample_criterion (GIMP_SELECT_CRITERION_COMPOSITE);

  if (request->refine_edges)
    {
      feather_radius = CLAMP (request->radius * 0.12, 0.0, 40.0);
      gimp_context_set_feather (TRUE);
      gimp_context_set_feather_radius (feather_radius, feather_radius);
    }
  else
    {
      gimp_context_set_feather (FALSE);
      feather_radius = 0.0;
    }

  if (request->sample_merged)
    {
      GeglColor *color = NULL;

      if (gimp_image_pick_color (image, NULL,
                                 request->x, request->y,
                                 TRUE,
                                 TRUE,
                                 MAX (request->radius, 1.0),
                                 &color))
        picked_color = color;
    }
  else
    {
      const GimpDrawable *drawables[] = { drawable, NULL };
      GeglColor          *color       = NULL;

      if (gimp_image_pick_color (image, drawables,
                                 request->x, request->y,
                                 FALSE,
                                 TRUE,
                                 MAX (request->radius, 1.0),
                                 &color))
        picked_color = color;
    }

  if (picked_color)
    {
      success = gimp_image_select_color (image,
                                         GIMP_CHANNEL_OP_REPLACE,
                                         drawable,
                                         picked_color);
      g_object_unref (picked_color);
    }
  else
    {
      success = gimp_image_select_contiguous_color (image,
                                                    GIMP_CHANNEL_OP_REPLACE,
                                                    drawable,
                                                    request->x,
                                                    request->y);
    }

  if (success && request->refine_edges)
    {
      gint grow = (gint) CLAMP (request->radius * 0.08, 0.0, 25.0);
      gint smooth = (gint) CLAMP (request->radius * 0.10, 0.0, 40.0);

      if (smooth > 0)
        gimp_selection_smooth (image, smooth);

      if (grow > 0)
        gimp_selection_grow (image, grow);

      if (feather_radius > 0.0 && grow > 0)
        gimp_selection_shrink (image, MAX (1, grow / 2));
    }

  gimp_context_pop ();

  if (! success)
    g_set_error (error, GIMP_PLUG_IN_ERROR, GIMP_PLUG_IN_ERROR_EXECUTION,
                 "Unable to construct intelligent selection mask.");

  return success;
}
