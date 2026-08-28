#include "nysor/block_loader.hpp"

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

}  // namespace

BlockKind parse_operation(const std::string& operation) {
  if (operation == "add") return BlockKind::Add;
  if (operation == "subtract") return BlockKind::Subtract;
  if (operation == "multiply") return BlockKind::Multiply;
  if (operation == "divide") return BlockKind::Divide;
  throw std::invalid_argument("Unsupported operation: " + operation +
                              ". Supported operations: add, subtract, multiply, divide");
}

BlockDefinition load_block(const std::filesystem::path& path) {
  const Json json = read_json(path);
  BlockDefinition definition;
  definition.name = required_string(json, path, "name");
  definition.version = required_string(json, path, "version");
  definition.kind = json.contains("kind") ? required_string(json, path, "kind") : "binary";
  definition.operation = required_string(json, path, "operation");
  definition.input_count = required_count(json, path, "inputs");
  definition.output_count = required_count(json, path, "outputs");

  if (definition.kind != "binary") {
    throw manifest_error(path, "kind", "Unsupported kind: " + definition.kind);
  }
  try {
    parse_operation(definition.operation);
  } catch (const std::invalid_argument& error) {
    throw manifest_error(path, "operation", error.what());
  }
  if (definition.input_count != 2) {
    throw manifest_error(path, "inputs", "Binary blocks require exactly 2 inputs");
  }
  if (definition.output_count != 1) {
    throw manifest_error(path, "outputs", "Binary blocks require exactly 1 output");
  }
  return definition;
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
                              input.get<std::string>());
        }
      } else {
        for (const auto& [name, input] : block["inputs"].items()) {
          if (!input.is_string()) throw graph_error(path, "Node " + id + " input " + name + " must be a node id");
          node.inputs.emplace(name, input.get<std::string>());
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

Graph build_graph(const GraphDefinition& definition,
                  const std::filesystem::path& source_path) {
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
    const auto manifest_path = path.parent_path() / node.block;
    const auto block = load_block(manifest_path);
    if (node.inputs.size() != static_cast<std::size_t>(block.input_count)) {
      throw graph_error(path, "Node " + node.id + ": expected " +
                                   std::to_string(block.input_count) + " inputs, received " +
                                   std::to_string(node.inputs.size()));
    }
    if (!node.inputs.contains("left") || !node.inputs.contains("right")) {
      throw graph_error(path, "Node " + node.id + " requires left and right inputs");
    }
    for (const auto& [name, input] : node.inputs) {
      if (!indices.contains(input)) {
        throw graph_error(path, "Node " + node.id + " input " + name +
                                   " references unknown node: " + input);
      }
      const auto dependency = indices.at(input);
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
    const auto block = load_block(path.parent_path() / node.block);
    const auto operation = parse_operation(block.operation);
    node_ids[node.id] = graph.add_binary(operation,
                                         node_ids.at(node.inputs.at("left")),
                                         node_ids.at(node.inputs.at("right")));
  }
  graph.add_output(node_ids.at(definition.output));
  return graph;
}

Graph load_graph(const std::filesystem::path& path) {
  return build_graph(load_graph_definition(path), path);
}

}  // namespace nysor