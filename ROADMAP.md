## Nysor 0.7 — Block Packages

The 0.7 milestone establishes the first local package ecosystem. Packages are
portable, inspectable units that separate distribution metadata from the Block
specification and implementation details. The package registry provides stable
identity-based Block resolution, duplicate-ID protection, and simple dependency
compatibility checks. The verified pipeline is:

```text
Package -> Registry -> Block Resolution -> Graph -> IR -> Runtime
```

This milestone intentionally excludes remote discovery, marketplaces, native
library loading, and global dependency solving.

## Nysor 0.8 — Heterogeneous Blocks

The 0.8 milestone extends package resolution beyond arithmetic operations.
Blocks can declare named specifications such as `core.event` and compose
through compatible ports without the core knowing their domain semantics. The
initial proof uses Time, Event, State, and Print Blocks with deliberately small
runtime behavior. Adapters and dissolvers remain deferred.

## Nysor 0.9 — Connectors and Dissolvers

The 0.9 milestone makes graph resolution an opt-in transformation stage.
Directly equal specifications remain direct connections; otherwise the package
registry can discover unary Dissolver Blocks and find a deterministic BFS path
between specifications. The selected Blocks are inserted into a resolved graph
with conversion provenance before the existing compiler runs. Nysor core
contains no domain-specific conversion rules.
# Roadmap

Project Nysor is deliberately a design and research repository at the beginning of its lifecycle. The roadmap should be read as a staged research and engineering path rather than a promise of a complete engine.

## Phase 0 — Research and Architecture

The repository begins by defining terminology, documenting the goals of the engine, and exploring related systems.

Objectives:

- Define terminology.
- Research similar systems.
- Get feedback from engine developers.
- Draft the Block Standard.
- Identify fundamental technical problems.

## Phase 1 — Minimal Block Graph Prototype

Goal:

Prove that a graph of simple blocks can be represented, validated, scheduled, and executed.

Important constraints:

- No graphics engine required.
- No full editor required.
- Build a graph of simple pure blocks and simple stateful tests.

## Phase 2 — Graph Analysis

Add:

- dependency analysis;
- type checking;
- cycle detection;
- execution ordering.

The goal is to make validation more rigorous and expose graph structure in a way that is inspectable to developers.

## Phase 3 — Intermediate Representation

Convert validated block graphs into an internal representation that suits compiler and transformation work.

The IR should support:

- graph lowering;
- optimization passes;
- deterministic planning;
- backend mapping.

## Phase 4 — Optimization Experiments

Experiment with:

- constant folding;
- dead code elimination;
- block fusion;
- parallel execution.

The objective should be to benchmark graph optimization and execution against naive block interpretation and equivalent hand-written implementations.

## Phase 5 — Minimal Runtime

Create a minimal runtime capable of executing optimized block graphs.

The runtime should focus on:

- graph execution;
- ordering and dependency constraints;
- metadata inspection;
- scheduler and backend selection.

## Phase 6 — First Real Engine Experiment

Experiment with a very small 2D system.

For example:

```text
Transform
    ->
Velocity
    ->
Movement
```

This phase is intentionally limited. It should not attempt to build a full editor, marketplace, Vulkan renderer, physics engine, or AI system at the same time.

## Open Questions

The roadmap is meant to keep the project grounded. Open questions include:

- What is the first valid block graph that proves a useful scheduling model?
- Which optimization passes are worth committing to first?
- What level of runtime observability is necessary before a graph can be trusted?
- How should a first experiment approach CPU, GPU, and platform abstraction without prematurely overcommitting?
