#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace nysor {

using NodeId = std::size_t;

enum class BlockKind {
  Input,
  Constant,
  Time,
  Add,
  Subtract,
  Multiply,
  Divide,
  Sine,
  Event,
  State,
  Print,
  Output,
};

struct Node {
  NodeId id;
  BlockKind kind;
  std::vector<NodeId> inputs;
  double constant_value = 0.0;
};

class Graph {
 public:
  NodeId add_input(double value) {
    return add_node(BlockKind::Input, {}, value);
  }

  NodeId add_constant(double value) {
    return add_node(BlockKind::Constant, {}, value);
  }

  NodeId add_time(double value) {
    return add_node(BlockKind::Time, {}, value);
  }

  NodeId add_binary(BlockKind kind, NodeId left, NodeId right) {
    if (kind != BlockKind::Add && kind != BlockKind::Subtract &&
        kind != BlockKind::Multiply && kind != BlockKind::Divide) {
      if (kind == BlockKind::State) return add_node(kind, {left, right});
      throw std::invalid_argument("add_binary requires an arithmetic block");
    }
    return add_node(kind, {left, right});
  }

  NodeId add_output(NodeId input) {
    return add_node(BlockKind::Output, {input});
  }

  NodeId add_unary(BlockKind kind, NodeId input) {
    if (kind != BlockKind::Sine && kind != BlockKind::Event &&
      kind != BlockKind::Print) {
      throw std::invalid_argument("add_unary requires a supported unary block");
    }
    return add_node(kind, {input});
  }

  const std::vector<Node>& nodes() const { return nodes_; }

 private:
  NodeId add_node(BlockKind kind, std::vector<NodeId> inputs,
                  double constant_value = 0.0) {
    const NodeId id = nodes_.size();
    nodes_.push_back({id, kind, std::move(inputs), constant_value});
    return id;
  }

  std::vector<Node> nodes_;
};

}  // namespace nysor
