# Vision

Project Nysor explores an architecture for building software systems through composition. Game development is the primary initial application, but Nysor is not intended to be permanently restricted to games.

The central composition model is:

```text
Blocks -> Systems -> Engines -> Applications / Games
```

A Block can represent a mathematical operation, physics system, auto-aim system, tree, health bar, sound system, map, AI system, renderer, animation system, input system, or a larger engine. Small Blocks should be able to combine into larger Blocks while retaining the same broad abstraction.

```text
Primitive Blocks -> Composite Blocks -> Systems -> Engine -> Game
```

Nysor should support both curated collections for beginners and low-level composition for advanced developers. This is an architecture direction, not a claim that package collections or a beginner workflow already exist.

The central problem this project tries to solve is the mismatch between modern game requirements and the traditional engine model. Many engines provide a fixed stack of rendering, physics, animation, scenes, scripting, gameplay, audio, AI, and networking systems. That model is useful for large production pipelines, but it can become too rigid for highly specialized games, experimental projects, and people who want to reconfigure engine architecture rather than fit game behavior into a preselected abstraction.

The long-term vision is an engine architecture that lets developers choose the systems they need, arrange them as a block graph, inspect and validate the graph, and run the graph through analysis and optimization before execution. Major systems such as rendering, physics, animation, AI, gameplay, or audio can be represented as well-defined, reusable, and replaceable components. This is a move away from “one engine architecture for every project” and toward “a common engine core plus many architecture-building blocks.”

That philosophy is inspired by the relationship between a code editor and its ecosystem. A shared core provides editing, workspace structure, extensibility, and interoperability. Around the core, a vivid ecosystem of extensions and packages can add specialized capabilities. Project Nysor proposes a similar direction for game engines: a stable kernel, a block graph model, a formal or semi-formal Block Standard, and an ecosystem where developers can install, inspect, reuse, modify, and republish engine systems and libraries.

## Goals

The project aims to investigate:

- a small engine kernel with a formal responsibility boundary;
- graph-based composition for major engine systems;
- validation and optimization of system-level graphs;
- a block standard that can describe identity, inputs, outputs, types, state, dependencies, and execution metadata;
- extensible packages and native implementation boundaries;
- transparent and inspectable runtime workflows; and
- an AI-assisted design workflow that supports integration while avoiding control of deterministic execution.

## Non-Goals for the Initial Repository

This repository is not a production engine and does not implement the complete architecture. It intentionally focuses on documentation, design, a draft standard, and research. It is a research artifact meant to invite critique from engine developers, compiler developers, systems designers, and open-source contributors.

## Open Questions

Several important questions remain unresolved:

- How should a block ecosystem define compatibility and versioning?
- How much of the system should be written as portable graphs versus native extension code?
- How should the engine represent data ownership in a graph with many stateful and system blocks?
- What runtime guarantees should be required for deterministic execution?
- What is the minimum stable kernel needed for a truly modular engine architecture?
- How can the architecture avoid becoming too abstract for practical game development teams?

The answers to these questions are not yet decided in this repository.

## Status

**Current Prototype:** The repository contains a small C++ graph, validation, IR, dependency-analysis, scheduling, and Taskflow-backed execution prototype. It demonstrates scalar arithmetic graphs, not a complete general-purpose engine.

**Architecture Direction:** Nysor is being shaped as compiler and composition infrastructure. An editor should author graphs, libraries should collect Blocks, and future ecosystems should supply domain-specific meaning.

**Proposal / Future Work:** Composite Blocks, package collections, multiple ecosystems, flavours, and cross-domain software composition remain experimental directions.
