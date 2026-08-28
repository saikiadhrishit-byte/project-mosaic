# Compatibility

Universal compatibility is not a goal. Blocks should expose enough information for Nysor or an ecosystem to determine whether a connection is meaningful.

An electronics analogy is useful: components have inputs, outputs, voltage requirements, signal characteristics, and connectors. Components are not universally interchangeable, but compatible components can be connected, sometimes through an adapter.

```text
Block A Specification
        |
Connection Analysis
        |
Block B Specification
```

A connection may be:

- directly connectable;
- incompatible;
- potentially connectable through an adapter;
- unknown and requiring testing.

## Connectors and Adapters

Nysor should not require one universal connector semantics for every ecosystem. Ecosystems may define specifications and connectors suited to their domain. An adapter may itself be implemented as a Block.

```text
Block A -> Adapter / Connector Block -> Block B
```

## Dissolvers

“Dissolver” is the current conceptual name for a Block or mechanism that transforms, bridges, or integrates otherwise incompatible systems. The term and mechanism are not final.

Examples could include a physics adapter, a resource conversion Block, or a bridge between renderer representations. Dissolvers are not magical universal compatibility systems. Their makers are responsible for their semantics, behavior, tests, and compatibility claims.

Nysor should provide infrastructure to install, connect, execute, and test such components. It should not guarantee that a third-party Dissolver works universally.

## Responsibility Boundary

Nysor Core should provide composition infrastructure, diagnostics, and test execution. Block makers should provide implementations, specifications, compatibility claims, documentation, and relevant tests. A passing build or integration test is evidence for the tested conditions, not a universal compatibility guarantee.

The current prototype has scalar numeric operations only. Typed cross-ecosystem compatibility and adapters are architecture direction or future work.
