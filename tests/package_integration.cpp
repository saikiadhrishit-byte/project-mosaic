#include "nysor/package.hpp"
#include "nysor/runtime.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {

void write(const std::filesystem::path& path, const std::string& text) {
  std::ofstream file(path);
  if (!file) throw std::runtime_error("Could not create integration fixture");
  file << text;
}

std::filesystem::path package(const std::filesystem::path& root,
                              const std::string& id,
                              const std::string& operation) {
  const auto path = root / id;
  std::filesystem::create_directories(path);
  write(path / "package.json", "{\"id\":\"" + id + "\",\"version\":\"1.0.0\"}");
  write(path / "block.json", "{\"name\":\"" + operation + "\",\"version\":\"1.0.0\",\"kind\":\"binary\",\"operation\":\"" + operation + "\",\"inputs\":[{\"name\":\"left\",\"type\":\"number\"},{\"name\":\"right\",\"type\":\"number\"}],\"outputs\":[{\"name\":\"result\",\"type\":\"number\"}]}");
  return path;
}

void expect(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

}  // namespace

int main() {
  const auto root = std::filesystem::temp_directory_path() / "nysor_package_integration";
  std::filesystem::remove_all(root);
  try {
    nysor::PackageRegistry registry;
    registry.install(nysor::load_package(package(root, "core.math.constant", "add")));
    registry.install(nysor::load_package(package(root, "core.math.add", "add")));
    registry.install(nysor::load_package(package(root, "core.math.multiply", "multiply")));
    const auto graph_path = root / "graph.json";
    write(graph_path, R"({"nodes":[{"id":"result","block":"core.math.multiply","inputs":{"left":"sum","right":"multiplier"}},{"id":"sum","block":"core.math.add","inputs":{"left":"a","right":"b"}},{"id":"a","block":"constant","value":10},{"id":"b","block":"constant","value":5},{"id":"multiplier","block":"constant","value":2}],"output":"result"})");

    const auto definition = nysor::load_graph_definition(graph_path);
    const auto graph = nysor::build_graph(definition, registry, graph_path);
    expect(definition.nodes.size() == 5, "wrong discovered node count");
    expect(nysor::execute(nysor::lower_to_ir(graph)) == 30.0, "package graph result was not 30");

    const auto unknown_path = root / "unknown.json";
    write(unknown_path, R"({"nodes":[{"id":"result","block":"core.math.super_add","inputs":{"left":"a","right":"b"}},{"id":"a","block":"constant","value":10},{"id":"b","block":"constant","value":5}],"output":"result"})");
    try {
      nysor::build_graph(nysor::load_graph_definition(unknown_path), registry, unknown_path);
      throw std::runtime_error("unknown package was accepted");
    } catch (const std::out_of_range& error) {
      expect(std::string(error.what()).find("core.math.super_add") != std::string::npos,
             "unknown package was not rejected");
    }
    std::filesystem::remove_all(root);
    std::cout << "Nysor 0.7 Package -> Graph -> Runtime: PASS\n";
    return 0;
  } catch (const std::exception& error) {
    std::filesystem::remove_all(root);
    std::cerr << "FAILED: " << error.what() << '\n';
    return 1;
  }
}