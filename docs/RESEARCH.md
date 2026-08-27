# Research

Project Mosaic is an experimental research project focused on architecture, graph modeling, compiler-style analysis, and modular engine design. It should not pretend that any implementation has already solved all of the large technical problems identified below.

## Existing Technologies and Projects to Study

The following categories are of interest and should be considered as research material rather than direct references to an identical architecture.

### Game Engine Architecture

- engine kernels and subsystems
- engine runtime organization
- editor infrastructure
- game ECS and scene systems

### ECS

- entity-component systems
- data-oriented runtime patterns
- archetype storage and chunked storage
- entity model design

### Data-Oriented Design

- cache-friendly data layouts
- systems organized around collections and data flow
- message-free transformation pipelines

### Visual Programming Languages

- node-based systems
- graph editors
- visual scripting environments
- behavior authoring by composition

### Compiler Intermediate Representations

- SSA form
- IR design
- graph lowering
- typed instruction representations
- dataflow graphs

### Graph Compilers

- graph scheduling
- acyclic dependency graphs
- topological ordering
- graph analysis passes
- compiler backends

### GPU Compute

- compute shaders
- data-parallel execution models
- GPU scheduling and synchronization
- kernel dispatch patterns

### Vulkan

- graphics API structure
- pipelines
- resource barriers
- parallel command and synchronization model

### Physics Engines

- rigid-body simulation
- collision detection
- continuous and discrete integration
- constraints and soft-body systems

### Plugin Systems

- extension loading
- dynamic modules
- package metadata
- interoperability and ABI model

### Package Managers

- dependency resolution
- version constraints
- package metadata
- distribution and publication channels

## Questions to Research

The repository should not treat these as solved research questions:

- What are the most useful minimal invariants for a block graph?
- How can a graph compiler validate data flow before runtime?
- How should pure, stateful, system, event, and native blocks differ in metadata and scheduling?
- What information should a Block Standard require for compatibility and versioning?
- What level of type strictness is practical for game-engine graph authors?
- How should event-driven systems be represented within a mostly deterministic IR model?
- How should the engine coordinate CPU and GPU execution in the same graph?
- What are the responsibilities of the engine kernel versus the extension ecosystem?
- How should authoring, validation, and optimization remain understandable to developers?
- What limitations arise when every subsystem is made modular and composable?

## Open Questions

- How much of this research should use formal compiler concepts directly?
- How much of the graph model should remain visual versus declarative?
- Which categories of packages are appropriate to start with first?
- How will performance benchmarks be made fair between graph and hand-written implementations?
