# Compiler and Runtime

The long-term research direction for Project Nysor is to convert block graphs into an intermediate representation and analyze that representation before deciding how to execute it. This is the key idea behind the block-graph execution model: make the graph readable to developers, but also transform it into a structure that can be validated, optimized, and scheduled.

A block graph should not be treated only as a list of visual blocks that are interpreted in a fixed order at runtime. Instead, the graph should be examined as a whole. The analysis might reveal dependency ordering, type compatibility, shared data access, opportunities for fusion, and other optimizations that are difficult to perceive from a single node.

## Proposed Execution Pipeline

A high-level model is:

```text
Block Graph
    -> Validation
    -> Dependency Analysis
    -> Intermediate Representation
    -> Optimization
    -> Execution Plan
    -> CPU / GPU / Other Backend
```

This pipeline is conceptual and should be treated as a research plan rather than an implementation claim.

## Intermediate Representation

The intermediate representation should preserve enough information for:

- type checking;
- dependency analysis;
- cycle detection;
- scheduling;
- graph transformation;
- validation of side effects and data access patterns.

The IR may be an internal graph structure rather than a textual or visual language. It should be efficient for compiler-style passes and compatible with a small runtime or scheduler.

## Optimization Research

This repository explicitly treats optimization as experimental. Possible topics include:

- dependency analysis;
- type checking;
- cycle detection;
- dead code elimination;
- constant folding;
- block fusion;
- scheduling;
- parallel execution;
- memory and layout optimization;
- batching;
- CPU execution;
- GPU compute execution.

An example simple transformation might show a visual block sequence such as:

```text
Get Velocity
    ->
Multiply by Delta Time
    ->
Add to Position
    ->
Clamp Position
```

This graph could potentially be analyzed and lowered into an equivalent expression such as:

```text
Position = Clamp(Position + Velocity * DeltaTime)
```

That representation is a useful research direction, but it should not be mistaken for a final execution model.

## Runtime Responsibilities

A runtime should eventually coordinate:

- execution scheduling;
- memory access and lifetime boundaries;
- data synchronization;
- readiness and update ordering;
- event dispatch;
- cross-backend lowering;
- debugging and introspection.

The project will need to decide whether runtime responsibilities belong mostly in a kernel or in a scheduler and execution plan layer.

## Native and GPU Possibilities

The architecture should remain open to different backend execution strategies. A graph may be compiled into a CPU plan, a GPU compute representation, or a specialized native module. Native and GPU capabilities should be declared via metadata, but the execution strategy should be validated by the compiler and runtime.

The goal is not to force runtime work into one interpretation model. It is to make the graph understandable enough to support execution in the best available context.

## Open Questions

There are major unresolved questions:

- How portable should the IR be across CPU and GPU execution?
- How should blocks be fused safely without changing semantics?
- Should optimization be graph-wide or scoped by subsystem?
- How should nondeterministic or event-driven blocks influence scheduling?
- How can the compiler distinguish a logically pure block from a block that is “pure enough” for optimization but not side-effect free?
- What is the minimum evidence required before a compiler pass should be considered trustworthy?
