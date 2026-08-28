# Architecture

Project Nysor is described as an experimental, composable game-engine architecture. The repository documents a proposed layered execution and development model rather than a completed implementation.

The proposed flow is:

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

This architecture is an intentionally broad research direction. The goal is to make the engine architecture more modular and inspectable, while reaching for a representation that can be validated and optimized before runtime.

## Layered Interpretation

### 1. Developer / Editor

The user-facing layer includes the developer, editor experience, tooling, and authoring workflow. This layer provides the interface for creating a graph, attaching blocks, inspecting metadata, editing package dependencies, and collaborating with AI-assisted tools.

This layer is not necessarily a graphical editor in the first version. It may start with documentation, graph examples, and a small prototyping path.

### 2. Block Graph

A block graph is a collection of connected blocks describing a system or subsystem. Blocks represent an intent, a transformation, an event, a system operation, or a stateful construct. A graph should be readable by humans and analyzable by the compiler/runtime stack.

The initial graph model is intentionally simple. It allows blocks to be connected by inputs and outputs and grouped into a whole architecture plan.

### 3. Validation

Validation confirms that a graph is structurally correct. This includes checking that the graph is connected correctly, that required data flows exist, that block metadata matches declared interfaces, and that data types and outputs satisfy consumers.

Validation is a research area rather than a finished subsystem. It should establish a deterministic basis for all later compilation and optimization.

### 4. Intermediate Representation

The registry of blocks and the visual graph must be converted into an internal representation suitable for analysis. This intermediate representation may preserve semantics, show dependency structure, and permit graph passes that are not practical directly on the authored graph format.

The repository treats this layer as a major future area of research. It is a plausible direction for lowering the graph into a representation that can be optimized.

### 5. Optimization

The IR can be passed through optimization experiments. These may include dependency analysis, type checking, cycle detection, constant folding, dead code elimination, scheduling, memory/layout optimization, batched execution, and graph fusion. These are listed as research directions rather than claimed implementation steps.

Optimizations should also be explainable and transparent. A developer should be able to inspect why a graph has been optimized and which assumptions were made.

### 6. Scheduler / Execution Plan

A scheduler or execution planner determines the order of operations. It may schedule CPU work, GPU kernels, event responses, system batches, or stateful updates. The scheduler should consider data dependencies, synchronization, and parallelism.

This layer is a promising place for a deterministic runtime model that can be checked against the constraint of the original graph.

### 7. CPU / GPU / Other Backends

The system should eventually support different execution environments. A block graph could be compiled for CPU execution, GPU compute, network-ordered execution, or a specialized runtime. The architecture should not assume that all execution must run in one place.

## Kernel vs Systems

Project Nysor proposes a small kernel with a large and possibly open ecosystem of modules. A kernel should manage:

- graph representation and metadata;
- validation;
- reflection or introspection;
- a type model and package model;
- scheduling primitives;
- execution planning abstractions;
- memory and platform boundaries;
- package dependency resolution.

Major systems such as rendering, physics, animation, AI, audio, AI-assisted authoring, networking, and gameplay systems should be investigated as mode-specific modules or extensions. This design improves extensibility, but it introduces trade-offs: increased interface complexity, package compatibility concerns, dependency versioning, and the removal of engine-wide assumptions about component shape.

## Hypotheses and Future Research

The repository marks the following parts as hypotheses rather than completed architecture:

- single graph representation across all systems;
- validation and optimization passes that work across many domains;
- block fusion and transformation into a consistent IR;
- a formal block standard that can be used for package exchange;
- GPU execution paths based on the same graph structure;
- a practical marketplace for blocks and systems.

## Open Questions

- What is the absolute minimal kernel that remains worth maintaining?
- Which layers should be fully deterministic and which may remain more advisory or probabilistic?
- How should stateful blocks and system blocks be represented without forcing every block into a compute-only model?
- How should events and scheduling interact with a graph compiler?
- What is the right balance between transparency and optimization complexity?
- How can block graphs remain human-readable while also becoming executable plans?
