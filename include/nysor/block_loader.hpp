#pragma once

#include "nysor/graph.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace nysor {

class PackageRegistry;

struct PortDefinition {
  std::string name;
  std::string type;
  std::string specification;
};

enum class PortDirection {
  Input,
  Output,
};

struct ConnectionResult {
  bool compatible = false;
  std::string error;
};

struct PortConnection {
  std::string node;
  std::string port = "result";
};

struct BlockDefinition {
  std::string name;
  std::string version;
  std::string kind;
  std::string operation;
  int input_count = 0;
  int output_count = 0;
  std::vector<PortDefinition> input_ports;
  std::vector<PortDefinition> output_ports;
};

struct GraphNodeDefinition {
  std::string id;
  std::string block;
  double value = 0.0;
  std::unordered_map<std::string, PortConnection> inputs;
};

struct GraphDefinition {
  std::string version;
  std::vector<GraphNodeDefinition> nodes;
  std::string output;
};

BlockDefinition load_block(const std::filesystem::path& path);
BlockKind parse_operation(const std::string& operation);
const PortDefinition* find_port(const BlockDefinition& block,
                                PortDirection direction,
                                const std::string& name);
ConnectionResult validate_connection(const BlockDefinition& source,
                                     const std::string& source_port,
                                     const BlockDefinition& destination,
                                     const std::string& destination_port);
GraphDefinition load_graph_definition(const std::filesystem::path& path);
Graph build_graph(const GraphDefinition& definition,
                  const std::filesystem::path& source_path = {});
Graph build_graph(const GraphDefinition& definition,
                  const PackageRegistry& registry,
                  const std::filesystem::path& source_path = {});
Graph load_graph(const std::filesystem::path& path);

}  // namespace nysor