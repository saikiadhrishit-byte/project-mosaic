#pragma once

#include "nysor/package.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace nysor {

struct ConversionStep {
  std::string package_id;
  std::string input_port;
  std::string output_port;
};

struct ConversionPlan {
  bool direct = false;
  std::string source_specification;
  std::string destination_specification;
  int cost = 0;
  std::vector<ConversionStep> steps;
};

class ConversionError : public std::runtime_error {
public:
  ConversionError(std::string source_node, std::string source_port,
                  std::string source_specification, std::string destination_node,
                  std::string destination_port, std::string destination_specification,
                  std::string reason);

  const std::string& source_node() const { return source_node_; }
  const std::string& destination_node() const { return destination_node_; }
  const std::string& source_specification() const { return source_specification_; }
  const std::string& destination_specification() const { return destination_specification_; }

private:
  std::string source_node_;
  std::string source_port_;
  std::string source_specification_;
  std::string destination_node_;
  std::string destination_port_;
  std::string destination_specification_;
};

struct ConversionProvenance {
  std::string destination_node;
  std::string destination_port;
  std::string source_node;
  std::string source_port;
  ConversionPlan plan;
};

struct ResolvedGraphDefinition {
  GraphDefinition graph;
  std::vector<ConversionProvenance> conversions;
};

ConversionPlan plan_conversion(const BlockDefinition& source,
                               const std::string& source_port,
                               const BlockDefinition& destination,
                               const std::string& destination_port,
                               const PackageRegistry& registry);

ResolvedGraphDefinition resolve_conversions(const GraphDefinition& graph,
                                            const PackageRegistry& registry);

}  // namespace nysor