#include "nysor/runtime.hpp"

#include <cmath>
#include <iostream>

int main() {
  nysor::Graph graph;
  const auto ten = graph.add_constant(10.0);
  const auto five = graph.add_constant(5.0);
  const auto sum = graph.add_binary(nysor::BlockKind::Add, ten, five);
  const auto two = graph.add_constant(2.0);
  const auto product = graph.add_binary(nysor::BlockKind::Multiply, sum, two);
  graph.add_output(product);

  const auto validation = nysor::validate(graph);
  if (!validation.valid()) {
    for (const auto& error : validation.errors) {
      std::cerr << error << '\n';
    }
    return 1;
  }

  const double result = nysor::execute(nysor::lower_to_ir(graph));
  std::cout << "Output: " << result << '\n';
  return std::abs(result - 30.0) < 1e-9 ? 0 : 1;
}