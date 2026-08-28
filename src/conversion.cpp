#include "nysor/conversion.hpp"

#include <algorithm>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace nysor {

namespace {

std::string specification(const PortDefinition& port) {
  return port.specification.empty() ? port.type : port.specification;
}

std::string error_message(const std::string& source_specification,
                          const std::string& destination_specification,
                          const std::string& reason) {
  return "Conversion Error\n\nSource specification:\n" + source_specification +
         "\n\nDestination specification:\n" + destination_specification +
         "\n\nProblem:\n" + reason;
}

BlockDefinition constant_definition() {
  BlockDefinition definition;
  definition.name = "constant";
  definition.kind = "source";
  definition.output_count = 1;
  definition.output_ports = {{"result", "number", "number"}};
  return definition;
}

}  // namespace

ConversionError::ConversionError(std::string source_node,
                                 std::string source_port,
                                 std::string source_specification,
                                 std::string destination_node,
                                 std::string destination_port,
                                 std::string destination_specification,
                                 std::string reason)
    : std::runtime_error(error_message(source_specification, destination_specification, reason)),
      source_node_(std::move(source_node)), source_port_(std::move(source_port)),
      source_specification_(std::move(source_specification)),
      destination_node_(std::move(destination_node)), destination_port_(std::move(destination_port)),
      destination_specification_(std::move(destination_specification)) {}

ConversionPlan plan_conversion(const BlockDefinition& source,
                               const std::string& source_port,
                               const BlockDefinition& destination,
                               const std::string& destination_port,
                               const PackageRegistry& registry) {
  const auto* output = find_port(source, PortDirection::Output, source_port);
  const auto* input = find_port(destination, PortDirection::Input, destination_port);
  if (!output || !input) {
    throw ConversionError(source.name, source_port, output ? specification(*output) : "<unknown>",
                          destination.name, destination_port, input ? specification(*input) : "<unknown>",
                          "Referenced port does not exist");
  }
  const auto source_specification = specification(*output);
  const auto destination_specification = specification(*input);
  if (source_specification == destination_specification) {
    return {true, source_specification, destination_specification, 0, {}};
  }

  struct SearchNode {
    std::string current;
    std::vector<ConversionStep> steps;
    int cost = 0;
  };
  std::queue<SearchNode> pending;
  std::unordered_set<std::string> visited;
  pending.push({source_specification, {}, 0});
  visited.insert(source_specification);
  while (!pending.empty()) {
    auto current = std::move(pending.front());
    pending.pop();
    for (const auto& package_id : registry.dissolvers_from(current.current)) {
      const auto& package = registry.get(package_id);
      const auto& metadata = *package.block.conversion;
      if (!visited.insert(metadata.to).second) continue;
      auto steps = current.steps;
      steps.push_back({package_id, package.block.input_ports.front().name,
                       package.block.output_ports.front().name});
      const auto cost = current.cost + metadata.cost;
      if (metadata.to == destination_specification) {
        return {false, source_specification, destination_specification, cost, std::move(steps)};
      }
      pending.push({metadata.to, std::move(steps), cost});
    }
  }
  throw ConversionError("", source_port, source_specification, "", destination_port,
                        destination_specification, "No compatible conversion path exists");
}

ResolvedGraphDefinition resolve_conversions(const GraphDefinition& graph,
                                            const PackageRegistry& registry) {
  ResolvedGraphDefinition resolved{graph, {}};
  const auto original_node_count = graph.nodes.size();
  std::size_t insertion = 0;
  for (std::size_t index = 0; index < original_node_count; ++index) {
    if (resolved.graph.nodes[index].block == "constant") continue;
    const auto destination_id = resolved.graph.nodes[index].id;
    const auto destination_block = registry.get(resolved.graph.nodes[index].block).block;
    std::vector<std::string> input_names;
    for (const auto& [name, connection] : resolved.graph.nodes[index].inputs) input_names.push_back(name);
    std::sort(input_names.begin(), input_names.end());
    for (const auto& name : input_names) {
      const auto destination_port_name = name;
      const auto original_connection = resolved.graph.nodes[index].inputs.at(name);
      const auto source_index = std::find_if(
          resolved.graph.nodes.begin(), resolved.graph.nodes.begin() + original_node_count,
          [&original_connection](const GraphNodeDefinition& node) {
            return node.id == original_connection.node;
          });
      if (source_index == resolved.graph.nodes.begin() + original_node_count) {
        throw ConversionError(original_connection.node, original_connection.port, "<unknown>", destination_id, name,
                              "<unknown>", "Source node does not exist");
      }
      const auto& source_node = *source_index;
      const auto source_block = source_node.block == "constant"
                                    ? constant_definition()
                                    : registry.get(source_node.block).block;
      const auto& connection = original_connection;
      ConversionPlan plan;
      try {
        plan = plan_conversion(source_block, connection.port, destination_block, name, registry);
      } catch (const ConversionError& error) {
        throw ConversionError(source_node.id, connection.port, error.source_specification(),
                              destination_id, name, error.destination_specification(),
                              error.what());
      }
      if (plan.direct) continue;

      const auto original_source_node = connection.node;
      const auto original_source_port = connection.port;
      auto current_node = original_source_node;
      auto current_port = original_source_port;
      for (const auto& step : plan.steps) {
        const auto& dissolver = registry.get(step.package_id).block;
        GraphNodeDefinition inserted;
        inserted.id = "__nysor_dissolver_" + std::to_string(insertion++);
        inserted.block = step.package_id;
        inserted.inputs.emplace(step.input_port, PortConnection{current_node, current_port});
        resolved.graph.nodes.push_back(std::move(inserted));
        current_node = resolved.graph.nodes.back().id;
        current_port = step.output_port;
        (void)dissolver;
      }
      resolved.graph.nodes[index].inputs.at(name) = {current_node, current_port};
      resolved.conversions.push_back({destination_id, destination_port_name, original_source_node,
                                      original_source_port, plan});
    }
  }
  return resolved;
}

}  // namespace nysor