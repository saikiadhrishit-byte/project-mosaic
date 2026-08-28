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

nysor::BlockDefinition specification(std::string name,
                                     std::string input_type,
                                     std::string output_type) {
  return {std::move(name), "1.0.0", "native", "", 1, 1,
          {{"input", std::move(input_type)}},
          {{"output", std::move(output_type)}}};
}

}  // namespace

int main() {
  try {
    const auto add = nysor::load_block(root() / "examples/arithmetic/add.json");
    expect(nysor::find_port(add, nysor::PortDirection::Input, "left") != nullptr,
           "add.left was not found");
    expect(nysor::find_port(add, nysor::PortDirection::Input, "right") != nullptr,
           "add.right was not found");
    expect(nysor::find_port(add, nysor::PortDirection::Output, "result") != nullptr,
           "add.result was not found");
    expect(nysor::find_port(add, nysor::PortDirection::Input, "middle") == nullptr,
           "unknown port was found");
    std::cout << "Port lookup: PASS\nUnknown port rejection: PASS\n";

    expect(!nysor::validate_connection(add, "left", add, "right").compatible,
           "input to input connection was accepted");
    expect(!nysor::validate_connection(add, "result", add, "result").compatible,
           "output to output connection was accepted");
    expect(nysor::validate_connection(add, "result", add, "left").compatible,
           "output to input connection was rejected");
    std::cout << "Port direction validation: PASS\n";

    const auto transform_producer = specification("Producer", "game.transform", "game.transform");
    const auto transform_consumer = specification("Consumer", "game.transform", "game.transform");
    const auto physics_consumer = specification("PhysicsConsumer", "physics.transform", "game.transform");
    expect(nysor::validate_connection(transform_producer, "output",
                                      transform_consumer, "input").compatible,
           "matching specifications were rejected");
    const auto mismatch = nysor::validate_connection(transform_producer, "output",
                                                     physics_consumer, "input");
    expect(!mismatch.compatible && mismatch.error.find("game.transform") != std::string::npos &&
               mismatch.error.find("physics.transform") != std::string::npos,
           "mismatched specifications were accepted or poorly reported");
    std::cout << "Specification compatibility: PASS\n";

    expect(nysor::validate_connection(transform_producer, "output",
                                      transform_consumer, "input").compatible,
           "first fan-out connection failed");
    expect(nysor::validate_connection(transform_producer, "output",
                                      physics_consumer, "output").compatible == false,
           "invalid fan-out destination was accepted");
    std::cout << "Fan-out direction checks: PASS\n";
    std::cout << "Semantic mismatch rejected before execution: PASS\n";
              bool duplicate_rejected = false;
              try {
                     nysor::load_block(root() / "tests/data/duplicate_ports.json");
              } catch (const std::exception&) {
                     duplicate_rejected = true;
              }
              expect(duplicate_rejected, "duplicate input ports were accepted");
              duplicate_rejected = false;
              try {
                     nysor::load_block(root() / "tests/data/duplicate_input_output.json");
              } catch (const std::exception&) {
                     duplicate_rejected = true;
              }
              expect(duplicate_rejected, "duplicate input/output ports were accepted");
              std::cout << "Duplicate port rejection: PASS\n";
    std::cout << "NYSOR 0.6 PORT SYSTEM: PASS\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "FAILED: " << error.what() << '\n';
    return 1;
  }
}