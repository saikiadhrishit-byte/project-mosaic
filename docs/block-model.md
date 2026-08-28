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

The supported external operations include arithmetic, signal, heterogeneous
prototype operations, and the generic `dissolve` operation used by package-
provided unary Dissolvers. Inputs are positional node references. This is
intentionally a small experiment, not a finalized package or manifest system.

A Dissolver declares its conversion separately from its implementation:

```json
{
  "kind": "unary",
  "role": "dissolver",
  "operation": "dissolve",
  "inputs": [{"name": "input", "specification": "sensor.raw"}],
  "outputs": [{"name": "output", "specification": "consumer.value"}],
  "conversion": {"from": "sensor.raw", "to": "consumer.value", "cost": 1}
}
```

The 0.9 planner uses only the declared specification graph. Conversion behavior
belongs to the package implementation, not to Nysor core.

## Ports and Specifications

The current manifest loader supports explicit port specifications. A binary
arithmetic Block can declare named numeric inputs and outputs:

```json
{
  "inputs": [
    {"name": "left", "type": "number"},
    {"name": "right", "type": "number"}
  ],
  "outputs": [
    {"name": "result", "type": "number"}
  ]
}
```

During external graph composition, Nysor checks that each named connection is
declared by the Block manifest. Unknown ports are rejected before the existing
compiler receives the graph. This is current prototype behavior for numeric
arithmetic; the manifest format and general type model remain experimental.

Graph connections may explicitly identify both endpoints:

```json
{"left": {"node": "sum", "port": "result"}}
```

The older string form, such as `"left": "sum"`, remains supported as shorthand
for the `result` output port. The loader validates source output direction and
destination input names before compilation.

## External Composition

The current graph loader can compose constants and manifest-backed binary
Blocks from a separate graph JSON file. Node IDs may be referenced before the
referenced node appears in the file; the loader resolves all names and
topologically orders the result before constructing the existing core Graph.
The example in `examples/graphs/arithmetic_demo.json` intentionally places
`result` before its dependencies and still evaluates to `30`.

The first non-arithmetic experiment is `Time -> Sine -> Output`, defined by
`examples/signals/time.json`, `examples/signals/sine.json`, and
`examples/graphs/sine_demo.json`. It demonstrates that the external Block
pipeline is not limited to binary arithmetic, while remaining deliberately
small.

## Composite Blocks

Architecture direction allows a graph or collection of Blocks to become a larger Block. How nested graphs are represented, packaged, inspected, exported, and possibly flattened is future work.
