#include "nysor/block_loader.hpp"

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
    const auto block = nysor::load_block(root() / "tests/data/valid_add.json");
    expect(block.name == "add", "wrong block name");
    expect(block.version == "0.1.0", "wrong block version");
    expect(block.operation == "add", "wrong operation");
    expect(block.input_count == 2, "wrong input count");
    expect(block.output_count == 1, "wrong output count");
    expect(block.input_ports[0].name == "left" &&
           block.input_ports[1].name == "right",
         "wrong input port names");
    expect(block.output_ports[0].name == "result" &&
           block.output_ports[0].type == "number",
         "wrong output port specification");
    std::cout << "Valid block loading: PASS\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "FAILED: " << error.what() << '\n';
    return 1;
  }
}