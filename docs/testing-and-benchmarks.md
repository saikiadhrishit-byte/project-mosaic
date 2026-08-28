# Testing and Benchmarks

Compatibility should be demonstrated through repeatable build, initialization, execution, and integration tests. A single successful run is useful evidence, but it is not proof of reliable compatibility in every environment.

## Current Prototype

The repository currently tests:

- linear and branching arithmetic graphs;
- exact runtime results such as `30` and `35`;
- IR contents and execution levels;
- invalid arity and node references;
- missing `Output` closure;
- cycle detection;
- constant division by zero;
- Taskflow concurrency and dependency synchronization;
- a scheduler stress graph.
- valid and invalid JSON Block manifests;
- external graph composition loaded from `examples/graphs/simple.json`.
- out-of-order external graph composition from `examples/graphs/arithmetic_demo.json`.
- the non-arithmetic external `Time -> Sine -> Output` composition.
- the focused `tests/0.6/` port-system suite, including direction, type,
  duplicate-port, cycle, large-graph, and forward-reference cases.

The test suite also measures peak concurrency and task timing. It currently demonstrates that independent tasks can run concurrently and that dependent tasks start only after prerequisite completion.

## Maker Tests

A future Block package should be able to provide tests for initialization, basic execution, edge cases, and integration. The maker is stating the conditions under which the Block is claimed to work.

## Compatibility Tests

Multiple Blocks should eventually be testable as a combination. A report might distinguish:

```text
Build: PASS
Initialization: PASS
Execution: PASS
Integration Test: FAIL
```

Nysor should report observed results without claiming that a Block is universally compatible.

## Community Testing

Community-submitted compatibility results are a future ecosystem feature. They could accumulate evidence about tested combinations while distinguishing PASS, FAIL, and UNKNOWN results.

## Benchmarking

Standardized benchmarks are proposed future work. A benchmark should record workload, execution time, memory usage, CPU usage, peak concurrency, and relevant environment information such as CPU, GPU, RAM, operating system, Nysor version, and Block version.

Benchmarks should provide evidence rather than decide which Block is universally best.

## Conventional Comparison

The current prototype includes a small observational comparison between direct arithmetic and a Mosaic graph. It measures graph construction, validation, lowering, dependency analysis, and Taskflow execution. These measurements are exploratory and not a standardized benchmark. Tiny graphs are expected to have meaningful framework overhead compared with direct code.
