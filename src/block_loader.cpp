#include "nysor/block_loader.hpp"
#include "nysor/package.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <queue>
#include <unordered_map>

namespace nysor {

namespace {

using Json = nlohmann::json;

std::runtime_error manifest_error(const std::filesystem::path& path,
                                  const std::string& field,
                                  const std::string& problem) {
  return std::runtime_error("Block Manifest Error\n\nFile:\n" + path.string() +
                            "\n\nField:\n" + field + "\n\nProblem:\n" + problem);
}

std::runtime_error graph_error(const std::filesystem::path& path,
                               const std::string& problem) {
  return std::runtime_error("Graph Composition Error\n\nFile:\n" +
                            path.string() + "\n\nProblem:\n" + problem);
}

Json read_json(const std::filesystem::path& path) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Block Manifest Error\n\nFile:\n" + path.string() +
                             "\n\nProblem:\nFile does not exist or cannot be opened");
  }
  try {
    return Json::parse(file);
  } catch (const Json::parse_error& error) {
    throw manifest_error(path, "document", "Invalid JSON: " + std::string(error.what()));
  }
}

std::string required_string(const Json& json, const std::filesystem::path& path,
                            const char* field) {
  if (!json.contains(field) || !json[field].is_string() || json[field].get<std::string>().empty()) {
    throw manifest_error(path, field, "Required non-empty string is missing");
  }
  return json[field].get<std::string>();
}

int required_count(const Json& json, const std::filesystem::path& path,
                   const char* field) {
  if (!json.contains(field) || !json[field].is_number_integer() || json[field].get<int>() < 0) {
    throw manifest_error(path, field, "Expected a non-negative integer");
  }
  return json[field].get<int>();
}

std::vector<PortDefinition> parse_ports(const Json& json,
                                        const std::filesystem::path& path,
                                        const char* field) {
  if (!json.contains(field) || !json[field].is_array()) {
    throw manifest_error(path, field, "Expected an array of port definitions");
  }
  std::vector<PortDefinition> ports;
  for (const auto& port : json[field]) {
    if (!port.is_object() || !port.contains("name") || !port["name"].is_string() ||
        ((!port.contains("type") || !port["type"].is_string()) &&
         (!port.contains("specification") || !port["specification"].is_string()))) {
      throw manifest_error(path, field, "Each port requires a name and type or specification");
    }
    const auto name = port["name"].get<std::string>();
    if (std::any_of(ports.begin(), ports.end(),
                    [&name](const PortDefinition& existing) {
                      return existing.name == name;
                    })) {
      throw manifest_error(path, field, "Duplicate port name: " + name);
    }
    const auto specification = port.contains("specification")
                     ? port["specification"].get<std::string>()
                     : port["type"].get<std::string>();
    ports.push_back({name, specification, specification});
  }
  return ports;
}

}  // namespace

BlockKind parse_operation(const std::string& operation) {
  if (operation == "add") return BlockKind::Add;
  if (operation == "subtract") return BlockKind::Subtract;
  if (operation == "multiply") return BlockKind::Multiply;
  if (operation == "divide") return BlockKind::Divide;
  if (operation == "sine") return BlockKind::Sine;
  if (operation == "time") return BlockKind::Time;
  if (operation == "event") return BlockKind::Event;
  if (operation == "state") return BlockKind::State;
  if (operation == "print") return BlockKind::Print;
  throw std::invalid_argument("Unsupported operation: " + operation +
                              ". Supported operations: add, subtract, multiply, divide, sine, time, event, state, print");
}

BlockDefinition load_block(const std::filesystem::path& path) {
  const Json json = read_json(path);
  BlockDefinition definition;
  definition.name = required_string(json, path, "name");
  definition.version = required_string(json, path, "version");
  definition.kind = json.contains("kind") ? required_string(json, path, "kind") : "binary";
  definition.operation = required_string(json, path, "operation");
  if (json.contains("inputs") && json["inputs"].is_array()) {
    definition.input_ports = parse_ports(json, path, "inputs");
    definition.input_count = static_cast<int>(definition.input_ports.size());
  } else {
    definition.input_count = required_count(json, path, "inputs");
  }
  if (json.contains("outputs") && json["outputs"].is_array()) {
    definition.output_ports = parse_ports(json, path, "outputs");
    definition.output_count = static_cast<int>(definition.output_ports.size());
  } else {
    definition.output_count = required_count(json, path, "outputs");
  }

    if (definition.kind != "binary" && definition.kind != "unary" &&
      definition.kind != "source" && definition.kind != "state") {
    throw manifest_error(path, "kind", "Unsupported kind: " + definition.kind);
  }
  try {
    parse_operation(definition.operation);
  } catch (const std::invalid_argument& error) {
    throw manifest_error(path, "operation", error.what());
  }
  const int expected_inputs = (definition.kind == "binary" || definition.kind == "state") ? 2 :
                              definition.kind == "unary" ? 1 : 0;
  if (definition.input_count != expected_inputs) {
    if (definition.kind == "binary" || definition.kind == "state") {
      throw manifest_error(path, "inputs", "Binary blocks require exactly 2 inputs");
    } else if (definition.kind == "unary") {
      throw manifest_error(path, "inputs", "Unary blocks require exactly 1 input");
    } else {
      throw manifest_error(path, "inputs", "Source blocks require exactly 0 inputs");
    }
  }
  if (definition.output_count != 1) {
    throw manifest_error(path, "outputs", "Blocks require exactly 1 output");
  }
  if (definition.input_ports.empty()) {
    definition.input_ports = definition.kind == "binary"
                                 ? std::vector<PortDefinition>{{"left", "number", "number"},
                                                               {"right", "number", "number"}}
                                 : std::vector<PortDefinition>{{"input", "number", "number"}};
    if (definition.kind == "source") definition.input_ports.clear();
  }
  if (definition.output_ports.empty()) {
    definition.output_ports = {{"result", "number", "number"}};
  }
  for (const auto& input : definition.input_ports) {
    if (std::any_of(definition.output_ports.begin(), definition.output_ports.end(),
                    [&input](const PortDefinition& output) {
                      return output.name == input.name;
                    })) {
      throw manifest_error(path, "ports", "Duplicate port name across input/output: " + input.name);
    }
  }
  return definition;
}

const PortDefinition* find_port(const BlockDefinition& block,
                                PortDirection direction,
                                const std::string& name) {
  const auto& ports = direction == PortDirection::Input ? block.input_ports : block.output_ports;
  const auto port = std::find_if(ports.begin(), ports.end(),
                                 [&name](const PortDefinition& candidate) {
                                   return candidate.name == name;
                                 });
  return port == ports.end() ? nullptr : &*port;
}

ConnectionResult validate_connection(const BlockDefinition& source,
                                     const std::string& source_port,
                                     const BlockDefinition& destination,
                                     const std::string& destination_port) {
  const auto* output = find_port(source, PortDirection::Output, source_port);
  if (!output) {
    return {false, source.name + "." + source_port + " is not an output port"};
  }
  const auto* input = find_port(destination, PortDirection::Input, destination_port);
  if (!input) {
    return {false, destination.name + "." + destination_port + " is not an input port"};
  }
  const auto& output_specification = output->specification.empty() ? output->type : output->specification;
  const auto& input_specification = input->specification.empty() ? input->type : input->specification;
  if (output_specification != input_specification) {
    return {false, source.name + "." + source_port + " (" + output_specification + ") is incompatible with " +
                   destination.name + "." + destination_port + " (" + input_specification + ")"};
  }
  return {true, {}};
}

GraphDefinition load_graph_definition(const std::filesystem::path& path) {
  const Json json = read_json(path);
  const char* node_field = json.contains("nodes") ? "nodes" : "blocks";
  if (!json.contains(node_field) || !json[node_field].is_array()) {
    throw graph_error(path, "nodes must be an array");
  }
  if (!json.contains("output") || !json["output"].is_string()) {
    throw graph_error(path, "Required graph output is missing");
  }

  GraphDefinition definition;
  if (json.contains("version")) {
    if (!json["version"].is_string()) throw graph_error(path, "version must be a string");
    definition.version = json["version"].get<std::string>();
  }
  for (const auto& block : json[node_field]) {
    if (!block.is_object() || !block.contains("id") || !block["id"].is_string()) {
      throw graph_error(path, "Every node requires a string id");
    }
    const std::string id = block["id"].get<std::string>();
    GraphNodeDefinition node;
    node.id = id;
    if ((block.contains("type") && block["type"] == "constant") ||
      (block.contains("block") && block["block"] == "constant")) {
      if (!block.contains("value") || !block["value"].is_number()) {
        throw graph_error(path, "Node " + id + " constant requires a numeric value");
      }
      node.block = "constant";
      node.value = block["value"].get<double>();
    } else if (block.contains("type") && block["type"] == "source") {
      if (!block.contains("block") || !block["block"].is_string() ||
          !block.contains("value") || !block["value"].is_number()) {
        throw graph_error(path, "Source node " + id + " requires a block path and numeric value");
      }
      node.block = block["block"].get<std::string>();
      node.value = block["value"].get<double>();
    } else {
      if (!block.contains("block") || !block["block"].is_string()) {
        throw graph_error(path, "Node " + id + " requires a block manifest path");
      }
      node.block = block["block"].get<std::string>();
      if (!block.contains("inputs") ||
          (!block["inputs"].is_object() && !block["inputs"].is_array())) {
        throw graph_error(path, "Node " + id + " requires named inputs");
      }
      if (block["inputs"].is_array()) {
        if (block["inputs"].size() != 2) {
          throw graph_error(path, "Node " + id + " requires exactly 2 inputs");
        }
        for (std::size_t input_index = 0; input_index < 2; ++input_index) {
          const auto& input = block["inputs"][input_index];
          if (!input.is_string()) throw graph_error(path, "Node " + id + " input must be a node id");
          node.inputs.emplace(input_index == 0 ? "left" : "right",
                              PortConnection{input.get<std::string>(), "result"});
        }
      } else {
        for (const auto& [name, input] : block["inputs"].items()) {
          if (input.is_string()) {
            node.inputs.emplace(name, PortConnection{input.get<std::string>(), "result"});
          } else if (input.is_object() && input.contains("node") &&
                     input["node"].is_string() && input.contains("port") &&
                     input["port"].is_string()) {
            node.inputs.emplace(name, PortConnection{input["node"].get<std::string>(),
                                                      input["port"].get<std::string>()});
          } else {
            throw graph_error(path, "Node " + id + " input " + name +
                                       " must specify a node and port");
          }
        }
      }
    }
    if (std::any_of(definition.nodes.begin(), definition.nodes.end(),
                    [&id](const GraphNodeDefinition& existing) { return existing.id == id; })) {
      throw graph_error(path, "Duplicate node ID: " + id);
    }
    definition.nodes.push_back(std::move(node));
  }
  definition.output = json["output"].get<std::string>();
  return definition;
}

namespace {

Graph build_graph_impl(const GraphDefinition& definition,
                       const std::filesystem::path& source_path,
                       const PackageRegistry* registry) {
  const auto path = source_path.empty() ? std::filesystem::path("<graph>") : source_path;
  std::unordered_map<std::string, std::size_t> indices;
  for (std::size_t index = 0; index < definition.nodes.size(); ++index) {
    if (!indices.emplace(definition.nodes[index].id, index).second) {
      throw graph_error(path, "Duplicate node ID: " + definition.nodes[index].id);
    }
  }
  if (!indices.contains(definition.output)) {
    throw graph_error(path, "Output references unknown node: " + definition.output);
  }

  std::vector<std::size_t> dependencies(definition.nodes.size(), 0);
  std::vector<std::vector<std::size_t>> dependents(definition.nodes.size());
  for (std::size_t index = 0; index < definition.nodes.size(); ++index) {
    const auto& node = definition.nodes[index];
    if (node.block == "constant") continue;
    const auto block = registry ? registry->get(node.block).block
                  : load_block(path.parent_path() / node.block);
    if (node.inputs.size() != static_cast<std::size_t>(block.input_count)) {
      throw graph_error(path, "Node " + node.id + ": expected " +
                                   std::to_string(block.input_count) + " inputs, received " +
                                   std::to_string(node.inputs.size()));
    }
    for (const auto& [name, connection] : node.inputs) {
      const auto port = std::find_if(block.input_ports.begin(), block.input_ports.end(),
                                     [&name](const PortDefinition& candidate) {
                                       return candidate.name == name;
                                     });
      if (port == block.input_ports.end()) {
        throw graph_error(path, "Node " + node.id + " references unknown input port: " + name);
      }
    }
    if (block.kind == "binary" &&
        (!node.inputs.contains("left") || !node.inputs.contains("right"))) {
      throw graph_error(path, "Node " + node.id + " requires left and right inputs");
    }
    if (block.kind == "state" &&
        (!node.inputs.contains("set") || !node.inputs.contains("trigger"))) {
      throw graph_error(path, "Node " + node.id + " requires set and trigger inputs");
    }
    if (block.kind == "unary" && node.inputs.size() != 1) {
      throw graph_error(path, "Node " + node.id + " requires one input connection");
    }
    for (const auto& [name, connection] : node.inputs) {
      if (!indices.contains(connection.node)) {
        throw graph_error(path, "Node " + node.id + " input " + name +
                                   " references unknown node: " + connection.node);
      }
      const auto source = definition.nodes[indices.at(connection.node)];
      if (source.block == "constant") {
        if (connection.port != "result") {
          throw graph_error(path, "Node " + connection.node + "." + connection.port +
                                     " is not an output port");
        }
      } else {
        const auto source_block = registry ? registry->get(source.block).block
                   : load_block(path.parent_path() / source.block);
        const auto* source_port = find_port(source_block, PortDirection::Output, connection.port);
        if (!source_port) {
          throw graph_error(path, "Node " + connection.node + "." + connection.port +
                                     " is not an output port");
        }
        const auto* destination_port = find_port(block, PortDirection::Input, name);
        if (!destination_port || source_port->specification != destination_port->specification) {
          throw graph_error(path, "Node " + connection.node + "." + connection.port +
                                     " is incompatible with " + node.id + "." + name);
        }
      }
      const auto dependency = indices.at(connection.node);
      ++dependencies[index];
      dependents[dependency].push_back(index);
    }
  }
  std::queue<std::size_t> ready;
  for (std::size_t index = 0; index < dependencies.size(); ++index) {
    if (dependencies[index] == 0) ready.push(index);
  }
  std::vector<std::size_t> order;
  while (!ready.empty()) {
    const auto index = ready.front();
    ready.pop();
    order.push_back(index);
    for (const auto dependent : dependents[index]) {
      if (--dependencies[dependent] == 0) ready.push(dependent);
    }
  }
  if (order.size() != definition.nodes.size()) {
    throw graph_error(path, "Cyclic dependency detected");
  }

  Graph graph;
  std::unordered_map<std::string, NodeId> node_ids;
  for (const auto index : order) {
    const auto& node = definition.nodes[index];
    if (node.block == "constant") {
      node_ids[node.id] = graph.add_constant(node.value);
      continue;
    }
    const auto block = registry ? registry->get(node.block).block
                  : load_block(path.parent_path() / node.block);
    const auto operation = parse_operation(block.operation);
    if (block.kind == "source") {
      node_ids[node.id] = graph.add_time(node.value);
    } else if (block.kind == "unary") {
      const auto& input = node.inputs.begin()->second;
      node_ids[node.id] = graph.add_unary(operation,
                                          node_ids.at(input.node));
    } else if (block.kind == "state") {
      node_ids[node.id] = graph.add_binary(operation,
                                           node_ids.at(node.inputs.at("set").node),
                                           node_ids.at(node.inputs.at("trigger").node));
    } else {
      node_ids[node.id] = graph.add_binary(operation,
                                           node_ids.at(node.inputs.at("left").node),
                                           node_ids.at(node.inputs.at("right").node));
    }
  }
  graph.add_output(node_ids.at(definition.output));
  return graph;
}

}  // namespace

Graph build_graph(const GraphDefinition& definition,
                  const std::filesystem::path& source_path) {
  return build_graph_impl(definition, source_path, nullptr);
}

Graph build_graph(const GraphDefinition& definition,
                  const PackageRegistry& registry,
                  const std::filesystem::path& source_path) {
  return build_graph_impl(definition, source_path, &registry);
}

Graph load_graph(const std::filesystem::path& path) {
  return build_graph(load_graph_definition(path), path);
}

}  // namespace nysor