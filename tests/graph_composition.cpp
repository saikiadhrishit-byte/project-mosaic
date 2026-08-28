#include "nysor/block_loader.hpp"
#include "nysor/runtime.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace {

void expect(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

std::filesystem::path root() {
  return std::filesystem::path(__FILE__).parent_path().parent_path();
}

}  // namespace

int main() {
  try {
    const auto path = root() / "examples/graphs/arithmetic_demo.json";
    const auto definition = nysor::load_graph_definition(path);
    expect(definition.nodes.size() == 5, "wrong number of external graph nodes");
    const auto graph = nysor::build_graph(definition, path);
    expect(nysor::validate(graph).valid(), "composed graph failed validation");

    const auto ir = nysor::lower_to_ir(graph);
    const auto analysis = nysor::analyze_dependencies(ir);
    expect(analysis.execution_levels.size() == 4, "wrong composed graph levels");
    expect(analysis.execution_levels[0].size() == 3,
           "constants were not resolved as independent roots");
    expect(analysis.execution_levels[1].size() == 1,
           "sum was not resolved after its inputs");
    expect(analysis.execution_levels[2].size() == 1,
           "result was not resolved after sum and multiplier");
    expect(nysor::execute(ir) == 30.0, "composed graph result was not 30");

    try {
      nysor::load_graph(root() / "tests/data/invalid_port_graph.json");
      throw std::runtime_error("unknown port was accepted");
    } catch (const std::runtime_error& error) {
      expect(std::string(error.what()).find("unknown input port: middle") !=
                 std::string::npos,
             "unknown port error was not reported");
    }
    try {
      nysor::load_graph(root() / "tests/data/invalid_output_port_graph.json");
      throw std::runtime_error("unknown output port was accepted");
    } catch (const std::runtime_error& error) {
      expect(std::string(error.what()).find("a.wrong") != std::string::npos,
             "unknown output port error was not reported");
    }

    std::cout << "=== Nysor 0.5 External Graph Composition ===\n"
              << "Nodes discovered: 5\n"
              << "Resolving dependencies...\n"
              << "Level 0: a, b, multiplier\n"
              << "Level 1: sum\n"
              << "Level 2: result\n"
              << "Compiling: PASS\n"
              << "Executing: Result = 30\n"
              << "=== NYSOR 0.5 PASS ===\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "FAILED: " << error.what() << '\n';
    return 1;
  }
}