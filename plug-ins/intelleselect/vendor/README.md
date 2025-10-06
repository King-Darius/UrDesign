# IntelliSelect SAM2 vendor tree

This directory ships the open-source Segment-Anything Model v2 Python package
from https://github.com/facebookresearch/sam2 so that IntelliSelect can access
its inference utilities without requiring contributors to fetch the modules
manually. The upstream license is preserved in `LICENSE.SAM2`.

Model checkpoints are **not** included; place compatible weights inside
`sam2/checkpoints/` according to the upstream installation guide. When weights
are unavailable the IntelliSelect plug-in transparently falls back to its
heuristic selection routine, so the tool continues to function offline.
