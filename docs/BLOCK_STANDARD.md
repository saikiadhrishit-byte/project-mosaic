# Block Standard

This document is an initial draft specification for the idea of a Block Standard. It is intentionally not a final specification. The purpose is to define what a block might declare and how a block might participate in a graph, validation pass, package ecosystem, and runtime plan.

The long-term direction is a formal standard for describing block identity, inputs, outputs, type information, state, access patterns, dependencies, execution metadata, and parallelism metadata.

## Identity

A block should have a stable identity and a versioned definition. A minimal identity model should include:

- name
- unique identifier
- version
- author or vendor
- category
- human-readable description
- required language or implementation profile

A project may later require more advanced identity features such as provenance, licensing metadata, compatibility ranges, and package namespace information.

## Inputs

Input metadata should describe how a block consumes data:

- input name
- expected type
- requirement level, such as required or optional
- default value
- accepted ranges or constraints
- documentation string

Input metadata should be made available to the validator and the editor. It should also support a graph compiler that can check compatibility between connected blocks.

## Outputs

Output metadata should describe the data produced by the block:

- output name
- output type
- shape or collection semantics
- deterministic relationship to inputs when applicable
- documentation string

Output metadata allows the graph to be analyzed for dependency direction and data flow consistency.

## Types

The type system must be able to describe encoded data values and graph relationships. For example:

- scalar types
- vectors and matrices
- entities or handles
- event objects
- collection or stream types
- resource or component references

The type model should be extensible enough to support native or specialized block implementations. The type system is one area where the Block Standard will need to evolve with prototype experience.

## State

Stateful blocks maintain information between updates or executions. Examples include timers, velocity, rigid-body state, animation state, and event queues.

The Block Standard should record:

- whether the block is pure or stateful;
- the shape of persistent state;
- lifetime semantics;
- expected mutation boundaries;
- synchronization or ordering expectations.

## Data Access

A block should be able to declare what data it reads and what data it writes. A block may also declare whether it reads only input values, reads and writes a data object, or manipulates a collection.

Examples:

- data read
- data written
- entity read
- entity written
- resource read
- resource written

This metadata supports dependency analysis and scheduling.

## Side Effects

A block should declare the possibility of side effects. This is important for validation, graph optimization, testability, and safety.

Possible side effects include:

- filesystem access
- network operations
- entity creation or destruction
- event emissions
- external operation calls
- native code execution

A block should not be assumed side-effect free only because it appears pure from a mathematical perspective. Side effects must be declared where they are important for correctness or scheduling.

## Dependencies

A block may require other blocks, packages, libraries, interpreters, or platform features.

Dependencies should include:

- required blocks
- required packages
- required versions
- compatibility ranges
- platform constraints
- optional features and capabilities

The dependency model should eventually support package inspection, reproducible builds, and graph validation.

## Execution Metadata

Execution metadata describes how and when a block may run. It may include:

- pure or stateful properties
- deterministic or nondeterministic behavior
- CPU capable
- GPU capable
- execution frequency
- required stage of a loop or system update
- expected batching behavior

This metadata will need careful language because “deterministic” and “pure” are not always the same thing in a stateful game system.

## Parallelism Metadata

A block should be able to describe parallel execution opportunities:

- whether block instances can run independently;
- whether a block can be batched;
- whether synchronization is required between instances;
- whether the block is order-sensitive;
- whether it is safe to parallelize across entities or collections;
- whether it should be scheduled as a system batch.

This metadata is central to eventual compiler and scheduler design.

## Open Questions

The following questions remain unresolved in the draft standard:

- Should the standard be schema-first or semantics-first?
- Should blocks expose only compile-time metadata, or also runtime introspection metadata?
- How should native blocks and pure blocks be expressed in the same Block Standard?
- How should stateful systems declare ownership, mutability, and lifetime in a portable way?
- How strict should type compatibility be across packages and versions?
- How much metadata is necessary for a useful and secure package ecosystem without making blocks too heavy to author?
