# Ecosystem

Nysor Core should remain as small and stable as practical. Domain-specific meaning should mostly live in ecosystems, libraries, and Blocks rather than in a monolithic core.

```text
Nysor Core
├── Compiler and graph infrastructure
├── Dependency analysis
├── Execution infrastructure
├── Diagnostics
└── Package loading (architecture direction)
        |
    Ecosystems
        |
      Blocks
```

An ecosystem might focus on game development, robotics, simulation, data processing, or another domain. It may define its own Block libraries, specifications, connectors, adapters, runtime integrations, and collections. Nysor should not need to understand every internal domain concept.

## Publishers and Distribution

Individuals, communities, and companies should potentially be able to publish Blocks and collections. Open-source, source-visible, partially inspectable, and closed-source commercial distribution are possible future models. Licensing and enforcement details remain unresolved.

A marketplace is not part of the initial compiler core. A future marketplace might support discovery, installation, versions, documentation, compatibility evidence, benchmark results, and commercial distribution. Publisher labels such as verified, community, experimental, or unverified are only future concepts, not finalized policy.

## Flavours

A future Nysor installation may have a minimal Core plus optional flavours, such as a beginner game-development collection, 2D, 3D, open-source, or robotics flavour. A flavour should be understood as a curated set of Blocks, tools, defaults, and configurations rather than a separate implementation of Nysor.

## Beginner and Advanced Use

A beginner-oriented path might be:

```text
Install Core -> Choose a flavour -> Install a collection -> Compose -> Build
```

An advanced path might be:

```text
Install minimal Core -> Create Blocks -> Define specifications -> Create connectors -> Compose systems
```

These are architecture direction. The current repository contains a small C++ prototype and an experimental editor, not a package marketplace, flavour manager, or cross-language ecosystem.
