# Vision

Project Mosaic explores a game-engine architecture where the engine core is deliberately small, stable, and inspectable, while major engine systems are represented as blocks, graphs, modules, packages, and extension ecosystems.

The central problem this project tries to solve is the mismatch between modern game requirements and the traditional engine model. Many engines provide a fixed stack of rendering, physics, animation, scenes, scripting, gameplay, audio, AI, and networking systems. That model is useful for large production pipelines, but it can become too rigid for highly specialized games, experimental projects, and people who want to reconfigure engine architecture rather than fit game behavior into a preselected abstraction.

The long-term vision is an engine architecture that lets developers choose the systems they need, arrange them as a block graph, inspect and validate the graph, and run the graph through analysis and optimization before execution. Major systems such as rendering, physics, animation, AI, gameplay, or audio can be represented as well-defined, reusable, and replaceable components. This is a move away from “one engine architecture for every project” and toward “a common engine core plus many architecture-building blocks.”

That philosophy is inspired by the relationship between a code editor and its ecosystem. A shared core provides editing, workspace structure, extensibility, and interoperability. Around the core, a vivid ecosystem of extensions and packages can add specialized capabilities. Project Mosaic proposes a similar direction for game engines: a stable kernel, a block graph model, a formal or semi-formal Block Standard, and an ecosystem where developers can install, inspect, reuse, modify, and republish engine systems and libraries.

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
