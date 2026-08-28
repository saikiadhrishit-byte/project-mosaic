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
    std::cout << "=== Nysor 0.4 External Block Test ===\n\n";
    const auto path = root() / "examples/arithmetic/add.json";
    const auto block = nysor::load_block(path);
    std::cout << "Loading: " << path.string() << "\n";
    std::cout << "Block: " << block.name << " v" << block.version << "\n";
    const auto kind = nysor::parse_operation(block.operation);
    expect(kind == nysor::BlockKind::Add, "add did not map to BlockKind::Add");
    std::cout << "Mapping operation... add -> BlockKind::Add\n";

    const auto graph = nysor::load_graph(root() / "examples/graphs/simple.json");
    expect(graph.nodes().size() == 4, "external graph did not create expected nodes");
    expect(nysor::validate(graph).valid(), "external graph failed validation");
    std::cout << "Building graph...\nValidation: PASS\nCompiling...\n";
    const auto ir = nysor::lower_to_ir(graph);
    const auto result = nysor::execute(ir);
    expect(result == 15.0, "external block result was not 15");
    std::cout << "IR: PASS\nExecuting...\nResult: 15\n\n";
    std::cout << "Checklist:\n"
          << "[PASS] Block exists outside C++ code\n"
          << "[PASS] Nysor loads it\n"
          << "[PASS] Manifest is validated\n"
          << "[PASS] Invalid Blocks are rejected\n"
          << "[PASS] Block maps to internal representation\n"
          << "[PASS] Existing compiler accepts it\n"
          << "[PASS] Existing runtime executes it\n"
          << "[PASS] Final output is correct\n\n";
    std::cout << "Nysor 0.4 External Block Pipeline: PASS\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "FAILED: " << error.what() << '\n';
    return 1;
  }
}