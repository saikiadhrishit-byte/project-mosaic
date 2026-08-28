#pragma once

#include "nysor/graph.hpp"

#include <taskflow/taskflow.hpp>

#include <atomic>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <string>
#include <queue>
#include <thread>
#include <vector>

namespace nysor {

struct ValidationResult {
  std::vector<std::string> errors;

  bool valid() const { return errors.empty(); }
};

inline ValidationResult validate(const Graph& graph) {
  ValidationResult result;
  const auto& nodes = graph.nodes();

  for (const auto& node : nodes) {
    const std::size_t expected_inputs =
        (node.kind == BlockKind::Input || node.kind == BlockKind::Constant) ? 0 :
        node.kind == BlockKind::Output ? 1 : 2;
    if (node.inputs.size() != expected_inputs) {
      result.errors.push_back("node " + std::to_string(node.id) +
                              " has the wrong number of inputs");
    }
    for (const NodeId input : node.inputs) {
      if (input >= nodes.size()) {
        result.errors.push_back("node " + std::to_string(node.id) +
                                " references a missing input");
      } else if (input >= node.id) {
        result.errors.push_back("node " + std::to_string(node.id) +
                                " must reference an earlier node");
      }
    }
    if (node.kind == BlockKind::Divide && node.inputs.size() == 2 &&
      node.inputs[1] < nodes.size() && node.inputs[1] < node.id &&
      nodes[node.inputs[1]].kind == BlockKind::Constant &&
      nodes[node.inputs[1]].constant_value == 0.0) {
      result.errors.push_back("node " + std::to_string(node.id) +
                              " divides by zero");
    }
  }

  if (nodes.empty() || nodes.back().kind != BlockKind::Output) {
    result.errors.push_back("graph must end with an Output block");
  }
  return result;
}

struct Instruction {
  BlockKind kind;
  std::vector<NodeId> inputs;
  double constant_value = 0.0;
};

struct IR {
  std::vector<Instruction> instructions;
};

struct DependencyAnalysis {
  std::vector<std::pair<NodeId, NodeId>> edges;
  std::vector<NodeId> independent_nodes;
  std::vector<std::vector<NodeId>> execution_levels;
};

struct TaskflowExecution {
  double result = 0.0;
  std::size_t peak_concurrency = 0;
  long long elapsed_ms = 0;
  struct TaskTiming {
    long long started_us = 0;
    long long completed_us = 0;
  };
  std::vector<TaskTiming> timings;
};

inline std::vector<std::vector<NodeId>> schedule_levels(const IR& ir);
inline IR lower_to_ir(const Graph& graph) {
  const ValidationResult validation = validate(graph);
  if (!validation.valid()) {
    throw std::invalid_argument(validation.errors.front());
  }

  IR ir;
  for (const auto& node : graph.nodes()) {
    ir.instructions.push_back({node.kind, node.inputs, node.constant_value});
  }
  return ir;
}

inline DependencyAnalysis analyze_dependencies(const IR& ir) {
  DependencyAnalysis analysis;
  std::vector<std::size_t> dependency_count(ir.instructions.size(), 0);
  for (NodeId id = 0; id < ir.instructions.size(); ++id) {
    dependency_count[id] = ir.instructions[id].inputs.size();
    for (const NodeId input : ir.instructions[id].inputs) {
      if (input >= ir.instructions.size()) {
        throw std::invalid_argument("IR contains a missing dependency");
      }
      analysis.edges.emplace_back(input, id);
    }
  }
  for (NodeId id = 0; id < dependency_count.size(); ++id) {
    if (dependency_count[id] == 0) analysis.independent_nodes.push_back(id);
  }
  analysis.execution_levels = schedule_levels(ir);
  return analysis;
}

inline TaskflowExecution execute_with_taskflow_timed(
  const IR& ir, const DependencyAnalysis& analysis,
  const std::vector<std::chrono::milliseconds>& simulated_work) {
  if (analysis.execution_levels != schedule_levels(ir)) {
    throw std::invalid_argument("Nysor levels do not match the IR dependencies");
  }
  tf::Taskflow taskflow;
  std::vector<double> values(ir.instructions.size());
  std::vector<tf::Task> tasks;
  std::atomic<std::size_t> active_tasks = 0;
  std::atomic<std::size_t> peak_concurrency = 0;
  std::vector<TaskflowExecution::TaskTiming> timings(ir.instructions.size());
  tasks.reserve(ir.instructions.size());
  const auto start = std::chrono::steady_clock::now();

  for (NodeId id = 0; id < ir.instructions.size(); ++id) {
    tasks.push_back(taskflow.emplace([&, id] {
      const auto task_start = std::chrono::steady_clock::now();
        timings[id].started_us = std::chrono::duration_cast<std::chrono::microseconds>(
          task_start - start).count();
      const std::size_t active = ++active_tasks;
      std::size_t peak = peak_concurrency.load();
      while (active > peak &&
             !peak_concurrency.compare_exchange_weak(peak, active)) {
      }
      if (id < simulated_work.size() && simulated_work[id].count() > 0) {
        std::this_thread::sleep_for(simulated_work[id]);
      }
      const auto& instruction = ir.instructions[id];
      switch (instruction.kind) {
        case BlockKind::Input:
          values[id] = instruction.constant_value;
          break;
        case BlockKind::Constant:
          values[id] = instruction.constant_value;
          break;
        case BlockKind::Add:
          values[id] = values[instruction.inputs[0]] + values[instruction.inputs[1]];
          break;
        case BlockKind::Subtract:
          values[id] = values[instruction.inputs[0]] - values[instruction.inputs[1]];
          break;
        case BlockKind::Multiply:
          values[id] = values[instruction.inputs[0]] * values[instruction.inputs[1]];
          break;
        case BlockKind::Divide:
          values[id] = values[instruction.inputs[0]] / values[instruction.inputs[1]];
          break;
        case BlockKind::Output:
          values[id] = values[instruction.inputs[0]];
          break;
      }
          timings[id].completed_us = std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::steady_clock::now() - start).count();
      --active_tasks;
    }));
  }
  for (const auto& edge : analysis.edges) {
    tasks[edge.first].precede(tasks[edge.second]);
  }

  tf::Executor executor(2);
  executor.run(taskflow).wait();
  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start);
  return {values.back(), peak_concurrency.load(), elapsed.count(), std::move(timings)};
}

inline TaskflowExecution execute_with_taskflow_timed(
    const IR& ir, const DependencyAnalysis& analysis,
    std::chrono::milliseconds simulated_work = {}) {
  return execute_with_taskflow_timed(
      ir, analysis, std::vector<std::chrono::milliseconds>(ir.instructions.size(),
                                                           simulated_work));
}

inline double execute_with_taskflow(const IR& ir,
                                    const DependencyAnalysis& analysis) {
  return execute_with_taskflow_timed(ir, analysis).result;
}

inline double execute(const IR& ir) {
  return execute_with_taskflow(ir, analyze_dependencies(ir));
}

inline std::vector<std::vector<NodeId>> schedule_levels(const IR& ir) {
  std::vector<std::size_t> remaining(ir.instructions.size(), 0);
  std::vector<std::vector<NodeId>> dependents(ir.instructions.size());
  std::queue<NodeId> ready;
  for (NodeId id = 0; id < ir.instructions.size(); ++id) {
    remaining[id] = ir.instructions[id].inputs.size();
    for (const NodeId input : ir.instructions[id].inputs) {
      if (input >= ir.instructions.size()) {
        throw std::invalid_argument("IR contains a missing dependency");
      }
      dependents[input].push_back(id);
    }
    if (remaining[id] == 0) ready.push(id);
  }

  std::vector<std::vector<NodeId>> levels;
  while (!ready.empty()) {
    const std::size_t level_size = ready.size();
    levels.emplace_back();
    levels.back().reserve(level_size);
    for (std::size_t index = 0; index < level_size; ++index) {
      const NodeId id = ready.front();
      ready.pop();
      levels.back().push_back(id);
      for (const NodeId dependent : dependents[id]) {
        if (--remaining[dependent] == 0) ready.push(dependent);
      }
    }
  }
  std::size_t scheduled = 0;
  for (const auto& level : levels) scheduled += level.size();
  if (scheduled != ir.instructions.size()) {
    throw std::invalid_argument("IR contains a cyclic dependency");
  }

  return levels;
}

inline std::vector<NodeId> schedule(const IR& ir) {
  const auto levels = schedule_levels(ir);
  std::vector<NodeId> order;
  for (const auto& level : levels) {
    order.insert(order.end(), level.begin(), level.end());
  }
  return order;
}

inline std::string dump_ir(const IR& ir) {
  std::ostringstream output;
  output << "IR:\n";
  for (NodeId id = 0; id < ir.instructions.size(); ++id) {
    const auto& instruction = ir.instructions[id];
    output << id << ": ";
    switch (instruction.kind) {
      case BlockKind::Input: output << "INPUT " << instruction.constant_value; break;
      case BlockKind::Constant: output << "CONSTANT " << instruction.constant_value; break;
      case BlockKind::Add: output << "ADD"; break;
      case BlockKind::Subtract: output << "SUBTRACT"; break;
      case BlockKind::Multiply: output << "MULTIPLY"; break;
      case BlockKind::Divide: output << "DIVIDE"; break;
      case BlockKind::Output: output << "OUTPUT"; break;
    }
    output << '\n';
  }
  return output.str();
}

}  // namespace nysor
