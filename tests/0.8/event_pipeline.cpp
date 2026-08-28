#include "nysor/package.hpp"
#include "nysor/runtime.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {

void write(const std::filesystem::path& path, const std::string& text) {
  std::ofstream file(path);
  if (!file) throw std::runtime_error("fixture could not be created");
  file << text;
}

std::filesystem::path make_package(const std::filesystem::path& root,
                                   const std::string& id,
                                   const std::string& block) {
  const auto path = root / id;
  std::filesystem::create_directories(path);
  write(path / "package.json", "{\"id\":\"" + id + "\",\"version\":\"1.0.0\"}");
  write(path / "block.json", block);
  return path;
}

}  // namespace

int main() {
  const auto root = std::filesystem::temp_directory_path() / "nysor_08_pipeline";
  std::filesystem::remove_all(root);
  try {
    nysor::PackageRegistry registry;
    registry.install(nysor::load_package(make_package(root, "core.time",
        R"({"name":"Timer","version":"1.0.0","kind":"source","operation":"time","inputs":[],"outputs":[{"name":"event","specification":"core.event"}]})")));
    registry.install(nysor::load_package(make_package(root, "core.event",
        R"({"name":"Event","version":"1.0.0","kind":"unary","operation":"event","inputs":[{"name":"trigger","specification":"core.event"}],"outputs":[{"name":"fired","specification":"core.event"}]})")));
    registry.install(nysor::load_package(make_package(root, "core.state",
        R"({"name":"State","version":"1.0.0","kind":"state","operation":"state","inputs":[{"name":"set","specification":"number"},{"name":"trigger","specification":"core.event"}],"outputs":[{"name":"value","specification":"number"}]})")));
    registry.install(nysor::load_package(make_package(root, "core.print",
        R"({"name":"Print","version":"1.0.0","kind":"unary","operation":"print","inputs":[{"name":"value","specification":"number"}],"outputs":[{"name":"output","specification":"number"}]})")));
    const auto graph_path = root / "graph.json";
    write(graph_path, R"({"nodes":[{"id":"print","block":"core.print","inputs":{"value":{"node":"state","port":"value"}}},{"id":"state","block":"core.state","inputs":{"set":"value","trigger":{"node":"event","port":"fired"}}},{"id":"event","block":"core.event","inputs":{"trigger":{"node":"timer","port":"event"}}},{"id":"timer","type":"source","block":"core.time","value":7},{"id":"value","type":"constant","value":42}],"output":"print"})");
    const auto graph = nysor::build_graph(nysor::load_graph_definition(graph_path), registry, graph_path);
    const auto ir = nysor::lower_to_ir(graph);
    if (nysor::execute(ir) != 42.0) throw std::runtime_error("heterogeneous pipeline result was not 42");
    std::filesystem::remove_all(root);
    std::cout << "Timer -> Event -> State -> Print: PASS\n";
    return 0;
  } catch (const std::exception& error) {
    std::filesystem::remove_all(root);
    std::cerr << "FAILED: " << error.what() << '\n';
    return 1;
  }
}
