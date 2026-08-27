# AI Integration

Artificial intelligence may eventually participate in this engine architecture as an assistant, generator, explainer, adapter creator, and graph repair collaborator. However, the repository should clearly separate the role of AI from the deterministic execution path.

The basic model is:

```text
Developer / AI
    ->
Block Graph
    ->
Deterministic Validation
    ->
Compiler
    ->
Optimization
    ->
Runtime
```

This diagram states the intended direction clearly: AI assists the construction and reasoning around graphs, but it should not be treated as the source of deterministic execution or correctness guarantees. The block graph and its validation, compiler, optimizer, and runtime should remain the authoritative execution path.

## Proposed AI Responsibilities

AI systems may help with tasks such as:

- finding relevant blocks and packages;
- explaining systems and graphs;
- generating block graphs from natural language prompts;
- suggesting connections between compatible blocks;
- generating adapters between incompatible or partially compatible blocks;
- identifying invalid graph structure or repair suggestions;
- explaining validation and optimization errors;
- suggesting performance improvements.

These are only proposals and do not imply that the engine has a complete AI subsystem yet.

## Boundaries

AI should remain an assistant and integrator. It should not directly claim authority over deterministic correctness. A graph should still pass validation before it becomes an execution plan. A compiler or optimizer should still check transformations. A runtime should still provide evidence that a schedule is valid.

The repository treats AI as a useful authoring and analysis layer rather than as a substitute for the engine's deterministic reasoning pipeline.

## Risks

AI assistance introduces real risks:

- generated graphs may be invalid or incomplete;
- AI may invent metadata that conflicts with the graph;
- recommendations may obscure dependency constraints;
- AI can reduce accountability if it is trusted too early.

A safe architecture should keep AI clearly upstream of validation and downstream of explicit developer review.

## Open Questions

- How much graph information should be exposed to AI assistants?
- How should AI-generated graphs be validated and recorded?
- What evidence should be kept when AI suggests a block graph or optimization pass?
- Should the engine allow AI explanations to influence validation choices directly?
- How can AI-generated packages remain auditable and inspectable?
