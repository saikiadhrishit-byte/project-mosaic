# Project Mosaic

Project Mosaic is an experimental open-source game-engine architecture project exploring a different kind of engine foundation: a small, stable kernel with a large ecosystem of replaceable, composable, and inspectable systems.

The project describes an ambition often summarized as "The VS Code of Game Engines": not one fixed engine structure, but a shared core and a library of interchangeable blocks, systems, and packages that developers can assemble for different game domains.

This repository is intentionally in the design and research stage. It does not claim that a complete engine exists. It explains the vision, outlines the architecture, drafts an initial Block Standard, and collects research directions and technical questions for future prototypes.

## Modular Engine Philosophy

Most traditional game engines provide a fixed architecture that bundles rendering, physics, animation, audio, AI, and scripting into a single engine shape. Project Mosaic proposes the opposite direction:

- keep a small engine kernel;
- model major systems as modular blocks and extension packages;
- make graphs and systems inspectable, debuggable, and composable;
- validate and optimize the shape of a graph before execution;
- experiment with CPU, GPU, and other execution backends without freezing the architecture too early.

The goal is not to force every game into one predetermined engine structure. Instead, the engine should make it easier to assemble the right set of systems for the right kind of game.

## Proposed Architecture

```text
Developer / Editor
    ->
Block Graph
    ->
Validation
    ->
Intermediate Representation
    ->
Optimization
    ->
Scheduler / Execution Plan
    ->
CPU / GPU / Other Backends
```

## Repository Documentation

The documentation in this repository is meant to communicate the early architecture clearly enough for engine developers, compiler developers, and contributors to critique and improve it.

- [Vision](docs/VISION.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Block Standard](docs/BLOCK_STANDARD.md)
- [Compiler and Runtime](docs/COMPILER_AND_RUNTIME.md)
- [Extensions and Packages](docs/EXTENSIONS_AND_PACKAGES.md)
- [AI Integration](docs/AI_INTEGRATION.md)
- [Research](docs/RESEARCH.md)
- [Roadmap](ROADMAP.md)

## Status

Project Mosaic is an experimental architecture and research project. It is not a completed engine and should not be treated as one. This repository focuses on:

1. communicating the vision;
2. documenting open architectural questions;
3. drafting an initial Block Standard;
4. gathering references to related technologies;
5. creating a roadmap for proof-of-concept prototypes.

## Roadmap

The initial roadmap is intentionally conservative:

1. Research and architecture terminology.
2. Minimal block graph prototype without graphics.
3. Graph analysis and validation.
4. Intermediate representation experiments.
5. Optimization experiments.
6. Minimal runtime.
7. A first real engine experiment using a tiny 2D system.

## Contributing and Critique

We welcome technical criticism, design suggestions, and careful discussion about the architecture. This project is interested in productive disagreement when it clarifies the design.

Please read [CONTRIBUTING.md](CONTRIBUTING.md) and the community guidelines in [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).
