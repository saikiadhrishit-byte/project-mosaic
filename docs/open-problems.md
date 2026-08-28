# Open Problems

Nysor is an experimental architecture. The following problems are intentionally unresolved and should not be presented as solved requirements.

1. The exact Block specification.
2. The exact meaning and structure of types and specifications.
3. Cross-language and cross-runtime interoperability.
4. Resource ownership and memory lifetime.
5. Runtime ABI design.
6. Sandboxing and security of third-party Blocks.
7. Version compatibility and dependency resolution.
8. How composite Blocks are packaged and exported.
9. Benchmark standardization.
10. Reliable compatibility testing.
11. Marketplace trust and moderation.
12. Performance costs of adapters, connectors, and Dissolvers.
13. How the compiler optimizes compositions of Blocks.
14. How nested Blocks are represented and potentially flattened.
15. How editable and closed-source Blocks coexist.
16. Long-term stability and backwards compatibility.

Additional open questions include how stateful, event-driven, GPU, native, and remote Blocks should participate in one composition model; how much metadata is necessary for useful analysis; and which guarantees the Core can make about third-party implementations.

The current prototype provides evidence for a much smaller question: whether a simple graph can be validated, lowered into an IR, analyzed into dependency levels, and executed with a Taskflow-backed runtime. It does not resolve the problems above.
