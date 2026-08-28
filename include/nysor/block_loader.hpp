#pragma once

#include "nysor/graph.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace nysor {

struct BlockDefinition {
  std::string name;
  std::string version;
  std::string kind;
  std::string operation;
  int input_count = 0;
  int output_count = 0;
};

struct GraphNodeDefinition {
  std::string id;
  std::string block;
  double value = 0.0;
  std::unordered_map<std::string, std::string> inputs;
};

struct GraphDefinition {
  std::string version;
  std::vector<GraphNodeDefinition> nodes;
  std::string output;
};

BlockDefinition load_block(const std::filesystem::path& path);
BlockKind parse_operation(const std::string& operation);
GraphDefinition load_graph_definition(const std::filesystem::path& path);
Graph build_graph(const GraphDefinition& definition,
                  const std::filesystem::path& source_path = {});
Graph load_graph(const std::filesystem::path& path);

}  // namespace nysor