# IntelliSelect Tool and Backend

The IntelliSelect tool streams scribble strokes into a GEGL buffer and asks
an on-device inference backend to produce a binary mask. The backend is kept
in `plug-ins/intelliselect/` and currently works with models distributed with
GIMP under `intelliselect/models`. Users can override the lookup path by
setting the `GIMP_INTELLISELECT_MODEL_PATH` environment variable.

## Local model management

Models are regular ONNX or TensorFlow Lite files. To add custom models place
them under `${GIMP_INTELLISELECT_MODEL_PATH:-$GIMP_CONFIG_HOME/intelliselect}`
and use the *Intelligent Select* preferences to choose the identifier. The
backend resolves the identifier by concatenating the directory and the
filename, allowing multiple variants (CPU/GPU) to be stored side by side.

The backend prefers the hardware backend selected in preferences. When the
backend string is `auto` it will fall back to CPU execution. Third parties can
contribute additional runners by dropping shared objects inside the model
folder and naming them `<model>-<backend>.so`.

During inference the tool displays a `GimpProgress` dialog. Model discovery and
loading are cached so repeated invocations reuse the same resources. The
backend interface is intentionally lightweight to allow future replacements
with platform specific accelerators.

## Testing

The `plug-ins/intelliselect/tests/` directory contains GLib based unit tests
that mock inference results by running the reference backend on small GEGL
buffers. The tests confirm that returned masks match the provided scribbles and
that applying the mask updates the mock selection buffer.
