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

The current loader supports the minimal manifest fields shown above for binary arithmetic Blocks. It maps `operation` values such as `add` and `multiply` to the internal C++ `BlockKind` without exposing C++ names in JSON. An external graph definition can reference constants and manifest paths, and the loader builds the equivalent Nysor `Graph` for the existing compiler and runtime.

The supported external operations are currently `add`, `subtract`, `multiply`, and `divide`. Inputs are positional node references. This is intentionally a small experiment, not a finalized package or manifest system.

## External Composition

The current graph loader can compose constants and manifest-backed binary
Blocks from a separate graph JSON file. Node IDs may be referenced before the
referenced node appears in the file; the loader resolves all names and
topologically orders the result before constructing the existing core Graph.
The example in `examples/graphs/arithmetic_demo.json` intentionally places
`result` before its dependencies and still evaluates to `30`.

## Composite Blocks

Architecture direction allows a graph or collection of Blocks to become a larger Block. How nested graphs are represented, packaged, inspected, exported, and possibly flattened is future work.
