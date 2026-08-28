# Block Model

A Block is Nysor's fundamental composable unit. Nysor should define the common contract as minimally as practical and avoid embedding the meaning of every domain concept into the core. An ecosystem may define what its physics body, Entity, renderer, robot, or resource means.

A conceptual Block contains:

```text
Block
├── Identity
├── Inputs
├── Outputs
├── Runtime Requirements
├── Dependencies
├── Memory / Data Requirements
└── Implementation
```

This is an architecture direction, not a finalized schema.

## Proposed Manifest

JSON is the proposed initial human-readable manifest format. The following is illustrative and experimental; it is not the final Nysor specification.

```json
{
  "name": "character_controller",
  "publisher": "example.studio",
  "version": "1.0.0",
  "runtime": {
    "abi": "nysor-v1",
    "platforms": ["windows", "linux"]
  },
  "inputs": [
    {"name": "input", "type": "example.input.state"},
    {"name": "physics", "type": "example.physics.body"}
  ],
  "outputs": [
    {"name": "transform", "type": "example.transform"}
  ],
  "dependencies": [],
  "implementation": {
    "type": "native",
    "library": "character.dll"
  }
}
```

The exact identity rules, type syntax, dependency format, implementation loading, versioning, and ABI are still open.

## Current Prototype

The current code does not load manifests. Its Block model is represented directly by C++ node kinds and scalar values. Inputs are positional node references, and the supported operations are the arithmetic nodes documented in [Architecture](architecture.md).

## Composite Blocks

Architecture direction allows a graph or collection of Blocks to become a larger Block. How nested graphs are represented, packaged, inspected, exported, and possibly flattened is future work.
