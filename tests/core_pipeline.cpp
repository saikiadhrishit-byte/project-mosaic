#include "nysor/runtime.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace {

void expect(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_block() {
  nysor::Graph graph;
  const auto input = graph.add_input(10.0);
  const auto amount = graph.add_constant(5.0);
  const auto add = graph.add_binary(nysor::BlockKind::Add, input, amount);
  const auto output = graph.add_output(add);
  expect(nysor::execute(nysor::lower_to_ir(graph)) == 15.0, "Add(5) failed");
  expect(output == 3, "unexpected graph node order");
}

void test_graph_and_validation() {
  nysor::Graph graph;
  const auto input = graph.add_input(10.0);
  const auto amount = graph.add_constant(5.0);
  graph.add_output(graph.add_binary(nysor::BlockKind::Add, input, amount));
  expect(graph.nodes().size() == 4, "graph construction failed");

  nysor::Graph invalid;
  const auto invalid_input = invalid.add_input(10.0);
  const auto add = invalid.add_binary(nysor::BlockKind::Add, invalid_input, 99);
  invalid.add_output(add);
  const auto validation = nysor::validate(invalid);
  expect(!validation.valid(), "missing connection was accepted");
}

void test_ir_and_runtime() {
  nysor::Graph graph;
  const auto input = graph.add_input(10.0);
  const auto five = graph.add_constant(5.0);
  const auto add = graph.add_binary(nysor::BlockKind::Add, input, five);
  const auto two = graph.add_constant(2.0);
  const auto multiply = graph.add_binary(nysor::BlockKind::Multiply, add, two);
  graph.add_output(multiply);
  const auto ir = nysor::lower_to_ir(graph);
  expect(nysor::dump_ir(ir).find("0: INPUT 10") != std::string::npos, "IR missing INPUT");
  expect(nysor::dump_ir(ir).find("2: ADD") != std::string::npos, "IR missing ADD");
  expect(nysor::execute(ir) == 30.0, "runtime result was not 30");
  expect(nysor::dump_ir(ir).find("5: OUTPUT") != std::string::npos,
      "IR missing OUTPUT");
}

void test_scheduler_and_branching() {
  nysor::Graph graph;
  const auto input = graph.add_input(10.0);
  const auto two = graph.add_constant(2.0);
  const auto five = graph.add_constant(5.0);
  const auto doubled = graph.add_binary(nysor::BlockKind::Multiply, input, two);
  const auto increased = graph.add_binary(nysor::BlockKind::Add, input, five);
  const auto output = graph.add_binary(nysor::BlockKind::Add, doubled, increased);
  graph.add_output(output);
  const auto ir = nysor::lower_to_ir(graph);
  const auto analysis = nysor::analyze_dependencies(ir);
  expect(analysis.edges.size() == 7, "dependency analysis found the wrong edges");
  expect(analysis.execution_levels.size() == 4,
      "branching graph produced the wrong Nysor levels");
  const auto order = nysor::schedule(ir);
  const auto position = [&](nysor::NodeId id) {
    return std::find(order.begin(), order.end(), id) - order.begin();
  };
  expect(position(input) < position(doubled), "scheduler ran before input");
  expect(position(input) < position(increased), "scheduler ran before input");
  expect(position(doubled) < position(output), "scheduler ran before doubled");
  expect(position(increased) < position(output), "scheduler ran before increased");
  expect(nysor::execute_with_taskflow(ir, analysis) == 35.0,
         "Taskflow execution result was not 35");
  std::cout << "Nysor -> Taskflow: dependency plan preserved, result = 35\n";
}

void test_execution_levels() {
  nysor::IR ir;
  ir.instructions = {
      {nysor::BlockKind::Constant, {}, 10.0},
      {nysor::BlockKind::Constant, {}, 20.0},
      {nysor::BlockKind::Add, {0, 1}, 0.0},
  };
  const auto levels = nysor::schedule_levels(ir);
  expect(levels.size() == 2, "scheduler produced the wrong number of levels");
  expect(levels[0] == std::vector<nysor::NodeId>{0, 1}, "A and B are not independent");
  expect(levels[1] == std::vector<nysor::NodeId>{2}, "C is not in the join level");
  std::cout << "Execution levels:\n\nLevel 0:\n  A\n  B\n\nLevel 1:\n  C\n";
}

void test_cycle_rejection() {
  nysor::IR cyclic;
  cyclic.instructions = {
      {nysor::BlockKind::Add, {1, 1}, 0.0},
      {nysor::BlockKind::Add, {0, 0}, 0.0},
  };
  bool rejected = false;
  try {
    nysor::schedule(cyclic);
  } catch (const std::invalid_argument&) {
    rejected = true;
  }
  expect(rejected, "cycle was accepted");
}

void test_task_execution_parallelism() {
  nysor::IR ir;
  ir.instructions = {
      {nysor::BlockKind::Constant, {}, 10.0},  // A
      {nysor::BlockKind::Constant, {}, 20.0},  // B
      {nysor::BlockKind::Add, {0, 1}, 0.0},    // C
      {nysor::BlockKind::Output, {2}, 0.0},    // D
  };
  const auto analysis = nysor::analyze_dependencies(ir);
  expect(analysis.execution_levels.size() == 3, "wrong A/B/C/D levels");
  expect(analysis.execution_levels[0] == std::vector<nysor::NodeId>{0, 1},
    "A and B were not scheduled together");
  expect(analysis.execution_levels[1] == std::vector<nysor::NodeId>{2},
    "C was not scheduled after A and B");
  expect(analysis.execution_levels[2] == std::vector<nysor::NodeId>{3},
    "D was not scheduled after C");

  const std::vector work = {
      std::chrono::milliseconds(100), std::chrono::milliseconds(150),
      std::chrono::milliseconds(100), std::chrono::milliseconds(50)};
  const auto parallel = nysor::execute_with_taskflow_timed(ir, analysis, work);
  const auto sequential_start = std::chrono::steady_clock::now();
  for (const auto duration : work) std::this_thread::sleep_for(duration);
  const auto sequential_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - sequential_start).count();

  expect(parallel.result == 30.0, "parallel execution result was not 30");
  expect(parallel.peak_concurrency >= 2,
    "Taskflow did not execute A and B concurrently");
  expect(parallel.timings[0].completed_us < parallel.timings[2].started_us,
      "C started before A completed");
  expect(parallel.timings[1].completed_us < parallel.timings[2].started_us,
      "C started before B completed");
  expect(parallel.timings[2].completed_us < parallel.timings[3].started_us,
      "D started before C completed");
  expect(parallel.elapsed_ms < sequential_ms,
    "parallel execution did not beat the sequential baseline");
  std::cout << "Task execution levels:\n\nLevel 0:\n  A\n  B\n\nLevel 1:\n  C\n\nLevel 2:\n  D\n";
  std::cout << "Task execution: peak concurrency = " << parallel.peak_concurrency
       << ", parallel = " << parallel.elapsed_ms << " ms, sequential = "
       << sequential_ms << " ms\n";
  std::cout << "Synchronization: A/B -> C -> D: PASS\n";
}

}  // namespace

int main() {
  try {
    std::cout << "[NYSOR] Creating blocks...\n";
    test_block();
    std::cout << "[NYSOR] Building graph...\n";
    test_graph_and_validation();
    std::cout << "[NYSOR] Validating graph...\n";
    std::cout << "[NYSOR] Validation: PASS\n";
    std::cout << "[NYSOR] Compiling graph -> IR...\n";
    test_ir_and_runtime();
    std::cout << "[NYSOR] IR: PASS\n";
    std::cout << "[NYSOR] Scheduling dependencies...\n";
    test_scheduler_and_branching();
    test_execution_levels();
    test_task_execution_parallelism();
    std::cout << "[NYSOR] Schedule: PASS\n";
    test_cycle_rejection();
    std::cout << "[NYSOR] Executing runtime...\n";
    std::cout << "[NYSOR] Result: 30\n\n";
    std::cout << "CORE PIPELINE: PASS\n";
  } catch (const std::exception& error) {
    std::cerr << "FAILED: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
