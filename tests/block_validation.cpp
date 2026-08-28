#include "nysor/block_loader.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace {

void expect_error(const std::filesystem::path& path, const char* expected) {
  try {
    nysor::load_block(path);
  } catch (const std::exception& error) {
    if (std::string(error.what()).find(expected) == std::string::npos) {
      throw std::runtime_error("unexpected manifest error");
    }
    return;
  }
  throw std::runtime_error("invalid manifest was accepted");
}

std::filesystem::path root() {
  return std::filesystem::path(__FILE__).parent_path().parent_path();
}

}  // namespace

int main() {
  try {
    const auto data = root() / "tests/data";
    if (nysor::parse_operation("add") != nysor::BlockKind::Add ||
        nysor::parse_operation("multiply") != nysor::BlockKind::Multiply) {
      throw std::runtime_error("operation mapping failed");
    }
    std::cout << "Operation mapping: PASS\n";
    try {
      nysor::parse_operation("banana");
      throw std::runtime_error("invalid operation was accepted");
    } catch (const std::invalid_argument&) {
      std::cout << "Invalid operation rejection: PASS\n";
    }
    expect_error(data / "invalid.json", "Invalid JSON");
    std::cout << "Invalid JSON: PASS\n";
    expect_error(data / "missing_fields.json", "version");
    std::cout << "Missing field validation: PASS\n";
    expect_error(data / "unsupported_operation.json", "Unsupported operation: banana");
    expect_error(data / "invalid_input_count.json", "exactly 2 inputs");
    std::cout << "Invalid input count rejection: PASS\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "FAILED: " << error.what() << '\n';
    return 1;
  }
}