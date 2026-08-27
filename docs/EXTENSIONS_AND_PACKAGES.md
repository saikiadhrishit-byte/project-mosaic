# Extensions and Packages

Project Mosaic imagines an extension ecosystem in which developers can share and compose engine systems, blocks, libraries, tools, native code, and project examples. The project is not only about a block graph but also about a package and extension philosophy that can operate safely and transparently.

The long-term ecosystem is meant to support:

- individual blocks;
- block libraries;
- complete systems;
- rendering modules;
- physics modules;
- gameplay systems;
- tools;
- editor extensions;
- examples and test scenes.

The extension story is important because the engine is intended to be modular and composable. A developer should be able to install a rendering package, inspect its block graph and metadata, swap in another rendering implementation, and potentially publish a new package based on the same core system.

## Package Philosophy

A package can be a portable collection of metadata and executable artifacts. It should likely include:

- valid block definitions;
- graph examples or graphs;
- native or compiled implementation metadata;
- dependency declarations;
- documentation;
- test examples;
- version metadata;
- compatibility constraints.

This repository intends a transparent and inspectable package model rather than a hidden black-box plugin model. A package should ideally reveal enough about its contents to support trust, debugging, compatibility research, and future reuse.

## Extension Examples

A package such as a platformer character controller could include:

- blocks;
- block graphs;
- native implementations;
- dependency declarations;
- example projects or scenes;
- documentation;
- tests.

This structure supports a durable ecosystem where reusable gameplay systems can be reused by inspection and modification rather than only by opaque installation.

## Marketplace Vision

The long-term ambition is an ecosystem where a developer can:

```text
Install
    ->
Use
    ->
Inspect
    ->
Modify
    ->
Extend
    ->
Republish
```

This workflow encourages transparency and engineering literacy. It also aims to avoid the worst problems of closed engine ecosystems: uninspectable internals, broken abstractions, and barriers to sharing knowledge.

## Trade-Offs and Risks

Making everything modular introduces clear design costs:

- packages require stable metadata;
- dependencies need clear lifecycle management;
- block compatibility rules need deep thought;
- system boundaries must be documented or they become vague;
- an open ecosystem can lead to poor quality or incompatible packages;
- a compiler and runtime must understand enough of the package model to remain trustworthy.

The repository acknowledges these trade-offs rather than pretending that modularity is automatically free.

## Open Questions

The packaging model currently has unresolved questions:

- What minimum metadata is required for a package to be inspectable and safe?
- Should packages carry source, compiled artifacts, or both?
- How do extension authors declare native code support for CPU, GPU, and platform boundaries?
- How should packages be versioned across a graph compiler and runtime ecosystem?
- How much of the ecosystem should be curated versus community managed?
- How should license information and provenance be represented in package metadata?
