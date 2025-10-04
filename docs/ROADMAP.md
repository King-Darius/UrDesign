# UrDesign multi-discipline roadmap

UrDesign aims to deliver a cohesive creative environment that unifies raster,
vector, layout, motion, and automation workflows. This living roadmap outlines
the pillars required to stand shoulder-to-shoulder with mature creative suites
while preserving the accessibility and openness of the project. Targets evolve
with community feedback, but the themes below provide long-term direction.

## Guiding principles

1. **Consistent interaction language** – shortcuts, gestures, handles, and
   popovers behave the same across tools so muscle memory transfers between
   disciplines.
2. **Non-destructive first** – every workflow should default to reversible
   edits, live previews, and stackable adjustments.
3. **Extensible core** – plug-ins, scripting, and data-driven panels must cover
   all pillars so contributors can iterate without patching the core binary.
4. **Collaborative by design** – documents, presets, and sessions are sharable
   and auditable, enabling teams to adopt UrDesign for production pipelines.

## Pillar breakdown

### Raster lab

| Focus | Goals | References |
| ----- | ----- | ---------- |
| Brush & paint system | Modernize dynamics, smear/blend engines, and preset
tags. Integrate palette management, stroke stabilization, and procedural brush
modes. | Krita's brush engines, MyPaint's dab pipelines |
| Non-destructive adjustments | Adjustment layers for tone, color, and HDR
pipelines. Scene-referred workflow with soft-proofing, LUT support, and history
states. | Darktable's scene-referred pipeline, GEGL graphs |
| Retouching & selections | Edge-aware selections, feather previews, and
destructive-free retouch tools. Emphasize GPU-accelerated healing and
frequency-split editing. | PhotoFlow masking, Natron roto tools |

### Vector studio

| Focus | Goals | References |
| ----- | ----- | ---------- |
| Shape system | Parametric primitives, boolean ops, corner widgets, and
variable stroke profiles. Provide contour/offset operations and shape libraries.
| Inkscape's Live Path Effects, Penpot's boolean engine |
| Text & typography | Text-on-path, linked text frames, OpenType feature
controls, variable fonts, and optical margin alignment. | Scribus typographic
engine, Pango layout |
| Precision tools | Numeric transform panels, snapping hierarchies, alignment
commands, and scale-aware grids with shortcuts. | Blender's snapping stack,
Inkscape guides |

### Layout board

| Focus | Goals | References |
| ----- | ----- | ---------- |
| Artboards & canvases | Multi-canvas documents, responsive presets, and
export slices with meta-data packaging. | Penpot artboards, Akira slice
workflows |
| Asset linking | Shared symbol instances, component overrides, and versioned
libraries. | Open source symbol systems (Akira, Penpot) |
| Review & collaboration | Comment layers, change tracking, and iteration
history. Integrate with portable session formats for design reviews. | Weblate
annotation flows |

### Motion & automation

| Focus | Goals | References |
| ----- | ----- | ---------- |
| Timeline & animation | Unified timeline panel for frame-by-frame, keyframe,
and tweened content. Support onion-skinning, easing curves, and audio
scrubbing. | OpenToonz timeline, Synfig interpolation |
| Procedural actions | Macro recorder, action stacks, and headless batch
runners. Provide JSON/task definitions for pipeline integration. | Krita's
recorded actions, Darktable styles |
| Scripting ecosystem | Sandbox Python, Lua, and JS bridges with reliable API
stability and runtime docs. Offer package manager for sharing scripts. |
Godot plug-in loaders, Blender scripting |

### Asset management

* Resource hub for brushes, palettes, templates, fonts, LUTs, and symbols.
* Version-aware packaging inspired by Krita's bundle format.
* Cloud-optional sync with diff-friendly `.urd` documents.

### Collaboration & review

* Presence indicators for co-editing sessions using CRDT-based syncing.
* Commenting, annotations, and approvals stored alongside document history.
* Exportable review decks (PDF, MP4, interactive HTML) for stakeholders.

## Milestones

### Foundation (current cycle)

* Modern transform overlay with proportional scaling defaults and rotation
  affordances.
* Unified preferences for handle scale, offsets, and behavior.
* Documentation revamp, contributor onboarding, and roadmap publication.

### Beta parity

* Adjustment layers, vector boolean operations, artboard exports, and timeline
  scaffolding.
* Non-destructive text styling, advanced snapping system, and component
  libraries.
* Procedural macros, headless render queue, and collaborative commenting.

### Production maturity

* CRDT-backed multi-user sessions with per-layer permissions.
* GPU-accelerated paint & filter engine with tile-based caching.
* Cross-discipline preset exchange, cloud sync, and extensible template packs.

## Contributing to the roadmap

Roadmap tickets live in the issue tracker under the `roadmap` label with linked
milestones. Contributors are encouraged to:

1. Pair design proposals with interaction mockups or UX benchmarks.
2. Prototype features in plug-ins or extension modules before core integration.
3. Document performance characteristics and regression tests alongside new
   systems.

Feedback threads on the community portal collect proposals for future revisions.
