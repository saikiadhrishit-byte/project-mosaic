# Project Nysor

Project Nysor is an experimental open-source game-engine architecture project exploring a different kind of engine foundation: a small, stable kernel with a large ecosystem of replaceable, composable, and inspectable systems.

The project describes an ambition often summarized as "The VS Code of Game Engines": not one fixed engine structure, but a shared core and a library of interchangeable blocks, systems, and packages that developers can assemble for different game domains.

This repository is intentionally in the design and research stage. It does not claim that a complete engine exists. It explains the vision, outlines the architecture, drafts an initial Block Standard, and collects research directions and technical questions for future prototypes.

## Modular Engine Philosophy

Most traditional game engines provide a fixed architecture that bundles rendering, physics, animation, audio, AI, and scripting into a single engine shape. Project Nysor proposes the opposite direction:

- keep a small engine kernel;
- model major systems as modular blocks and extension packages;
- make graphs and systems inspectable, debuggable, and composable;
- validate and optimize the shape of a graph before execution;
- experiment with CPU, GPU, and other execution backends without freezing the architecture too early.

The goal is not to force every game into one predetermined engine structure. Instead, the engine should make it easier to assemble the right set of systems for the right kind of game.

## Looking for Contributors

Project Nysor is currently an experimental, early-stage project, and we are looking for developers interested in exploring and challenging this architecture with us. 

Rather than building a predetermined engine design, the goal of this project is to **investigate whether a modular block-graph architecture actually works** for real-world game development, performance-critical rendering, and compilation pipelines.

You do not need to be an expert in game engines to contribute. The most valuable contributions right now are from people willing to experiment, question the architecture, and build small prototypes around these core ideas.

### Areas where help is especially welcome
- **C++ / systems programming** — core runtime and engine infrastructure
- **Vulkan / graphics programming** — rendering and backend experimentation
- **Compiler / IR design** — graph compilation and intermediate representations
- **Graph algorithms** — dependency analysis, validation and scheduling
- **Game-engine architecture** — ECS, data-oriented design and modular systems
- **Documentation** — explaining the architecture and creating examples
- **Testing / experimentation** — building small prototypes to validate or challenge design decisions

### Good places to start

If you're new to the project, start with issues labelled:
- `good first issue`
- `help wanted`
- `architecture`
- `documentation`

You do not need to understand the entire project before contributing. Many tasks are intentionally isolated experiments designed to explore and benchmark one specific part of the architecture.

### What we're looking for

We are especially interested in contributors who want to discuss and shape the architecture, not just implement features. 

Nysor is an experiment. If you think an architectural decision is wrong, that is highly valuable feedback—and a great opportunity to prototype a better approach.

If you are interested, open an issue or start a discussion describing what you would like to work on!

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

- [Nysor Documentation](docs/README.md)
- [Vision](docs/vision.md)
- [Architecture](docs/architecture.md)
- [Block Model](docs/block-model.md)
- [Compatibility](docs/compatibility.md)
- [Testing and Benchmarks](docs/testing-and-benchmarks.md)
- [Ecosystem](docs/ecosystem.md)
- [Open Problems](docs/open-problems.md)
- [Legacy Block Standard](docs/BLOCK_STANDARD.md)
- [Compiler and Runtime Notes](docs/COMPILER_AND_RUNTIME.md)
- [Extensions and Packages](docs/EXTENSIONS_AND_PACKAGES.md)
- [AI Integration](docs/AI_INTEGRATION.md)
- [Research](docs/RESEARCH.md)
- [Roadmap](ROADMAP.md)

## Status

Project Nysor is an experimental architecture and research project. It is not a completed engine and should not be treated as one. This repository focuses on:

1. communicating the vision;
2. documenting open architectural questions;
3. drafting an initial Block Standard;
4. gathering references to related technologies;
5. creating a roadmap for proof-of-concept prototypes.

## Current Milestone: Nysor 0.6 (Prototype Release)

Nysor 0.6 is a milestone release of the experimental prototype. The architecture remains experimental and open to developer feedback. Nysor now supports:

- External Block definitions
- External graph composition
- Explicit input/output ports
- Block specifications
- Port compatibility validation
- Dependency analysis
- Topological compilation
- Parallel execution
- Visual graph editing
- Build diagnostics

See the [Nysor architecture documentation](docs/architecture.md) for the current implementation boundary, architecture direction, and open problems.


## Roadmap

The initial roadmap is intentionally conservative:

1. Research and architecture terminology.
2. Minimal block graph prototype without graphics.
3. Graph analysis and validation.
4. Intermediate representation experiments.
5. Optimization experiments.
6. Minimal runtime.
7. A first real engine experiment using a tiny 2D system.

## Minimal C++ Prototype

The repository now includes a small Phase 1 prototype in `include/nysor` and
`examples/basic_graph.cpp`. It models scalar `Constant`, `Add`, `Subtract`,
`Multiply`, `Divide`, and `Output` blocks.

Its pipeline is intentionally direct:

```text
Block Graph -> Validation -> Tiny IR -> Taskflow Execution
```

The prototype validates input counts, forward references, divide-by-zero
constants, and the required final `Output` block. The lowered IR becomes a
Taskflow dependency graph, allowing independent blocks to be scheduled in
parallel. Optimization passes are deliberately absent at this stage.

Build it with CMake:

```text
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The optional `nysor_editor` target adds Dear ImGui, GLFW, and imnodes for a
small graph workspace. It depends on Nysor Core but Nysor Core does not
depend on the editor. Disable it with `-DNYSOR_BUILD_EDITOR=OFF` when building
headless or embedding the core elsewhere.

## Dependency Analysis Experiment

The core test also exercises the Nysor 0.2 boundary:

```text
Nysor Graph -> Dependency Analysis -> Nysor Execution Levels
                                      -> Taskflow Graph -> Taskflow Execution
```

Nysor owns the semantic edges and execution levels. The Taskflow adapter
consumes those analyzed edges to execute the graph; it does not decide graph
meaning or replace Nysor validation. The test verifies that independent
branches remain independent and that the Taskflow-backed result is `35`.

## Contributing and Critique

We welcome technical criticism, design suggestions, and careful discussion about the architecture. This project is interested in productive disagreement when it clarifies the design.

Please read [CONTRIBUTING.md](CONTRIBUTING.md) and the community guidelines in [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).
