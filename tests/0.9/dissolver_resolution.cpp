#include "nysor/conversion.hpp"
#include "nysor/runtime.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {

void expect(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void write(const std::filesystem::path& path, const std::string& text) {
  std::ofstream file(path);
  if (!file) throw std::runtime_error("fixture could not be created");
  file << text;
}

std::filesystem::path dissolver(const std::filesystem::path& root,
                                const std::string& id,
                                const std::string& from,
                                const std::string& to) {
  const auto path = root / id;
  std::filesystem::create_directories(path);
  write(path / "package.json", "{\"id\":\"" + id + "\",\"version\":\"1.0.0\"}");
  write(path / "block.json", "{\"name\":\"" + id + "\",\"version\":\"1.0.0\",\"kind\":\"unary\",\"role\":\"dissolver\",\"operation\":\"dissolve\",\"inputs\":[{\"name\":\"input\",\"specification\":\"" + from + "\"}],\"outputs\":[{\"name\":\"output\",\"specification\":\"" + to + "\"}],\"conversion\":{\"from\":\"" + from + "\",\"to\":\"" + to + "\"}}");
  return path;
}

nysor::BlockDefinition block(const std::string& name,
                              const std::string& input,
                              const std::string& output) {
  return {name, "1.0.0", "unary", "print", 1, 1,
          {{"input", input, input}}, {{"output", output, output}}};
}

}  // namespace

int main() {
  const auto root = std::filesystem::temp_directory_path() / "nysor_09_resolution";
  std::filesystem::remove_all(root);
  try {
    const auto source = block("Source", "", "A");
    const auto destination = block("Destination", "C", "C");
    nysor::PackageRegistry registry;
    registry.discover(dissolver(root, "z.a_to_b", "A", "B"));
    registry.discover(dissolver(root, "z.b_to_c", "B", "C"));
    const auto plan = nysor::plan_conversion(source, "output", destination, "input", registry);
    expect(!plan.direct && plan.steps.size() == 2, "multi-hop plan was not found");
    expect(plan.steps[0].package_id == "z.a_to_b" && plan.steps[1].package_id == "z.b_to_c",
           "multi-hop plan order was not deterministic");

    const auto direct = block("Direct", "A", "A");
    const auto direct_plan = nysor::plan_conversion(direct, "output", direct, "input", registry);
    expect(direct_plan.direct && direct_plan.steps.empty(), "direct connection inserted a dissolver");

    nysor::PackageRegistry selection;
    selection.discover(dissolver(root, "a.first", "A", "B"));
    selection.discover(dissolver(root, "b.second", "A", "B"));
    const auto selected = nysor::plan_conversion(source, "output", block("B", "B", "B"), "input", selection);
    expect(selected.steps.size() == 1 && selected.steps.front().package_id == "a.first",
           "BFS selection was not deterministic");

    nysor::PackageRegistry cyclic;
    cyclic.discover(dissolver(root, "cycle.a_to_b", "A", "B"));
    cyclic.discover(dissolver(root, "cycle.b_to_a", "B", "A"));
    bool no_path = false;
    try { nysor::plan_conversion(source, "output", block("Z", "Z", "Z"), "input", cyclic); }
    catch (const nysor::ConversionError& error) {
      no_path = std::string(error.what()).find("No compatible conversion path exists") != std::string::npos;
    }
    expect(no_path, "cyclic conversion graph did not terminate with an error");

    const auto invalid = root / "invalid";
    std::filesystem::create_directories(invalid);
    write(invalid / "package.json", "{\"id\":\"invalid\",\"version\":\"1.0.0\"}");
    write(invalid / "block.json", R"({"name":"invalid","version":"1.0.0","kind":"unary","role":"dissolver","operation":"dissolve","inputs":[{"name":"input","specification":"A"}],"outputs":[{"name":"output","specification":"C"}],"conversion":{"from":"A","to":"Z"}})");
    bool metadata_rejected = false;
    try { nysor::load_package(invalid); }
    catch (const std::runtime_error& error) {
      metadata_rejected = std::string(error.what()).find("Conversion metadata does not match") != std::string::npos;
    }
    expect(metadata_rejected, "inconsistent dissolver metadata was accepted");

        const auto producer_path = root / "package.sensor";
        std::filesystem::create_directories(producer_path);
        write(producer_path / "package.json", R"({"id":"package.sensor","version":"1.0.0"})");
        write(producer_path / "block.json", R"({"name":"Sensor","version":"1.0.0","kind":"source","operation":"time","inputs":[],"outputs":[{"name":"output","specification":"sensor.raw"}]})");
        const auto consumer_path = root / "package.consumer";
        std::filesystem::create_directories(consumer_path);
        write(consumer_path / "package.json", R"({"id":"package.consumer","version":"1.0.0"})");
        write(consumer_path / "block.json", R"({"name":"Consumer","version":"1.0.0","kind":"unary","operation":"print","inputs":[{"name":"input","specification":"consumer.value"}],"outputs":[{"name":"output","specification":"consumer.value"}]})");
        const auto first_bridge = dissolver(root, "package.bridge.one", "sensor.raw", "shared.value");
        const auto second_bridge = dissolver(root, "package.bridge.two", "shared.value", "consumer.value");
        nysor::PackageRegistry pipeline_registry;
        pipeline_registry.discover(producer_path);
        pipeline_registry.discover(consumer_path);
        pipeline_registry.discover(first_bridge);
        pipeline_registry.discover(second_bridge);
        const auto pipeline_path = root / "pipeline.json";
        write(pipeline_path, R"({"nodes":[{"id":"consumer","block":"package.consumer","inputs":{"input":{"node":"sensor","port":"output"}}},{"id":"sensor","type":"source","block":"package.sensor","value":9}],"output":"consumer"})");
        const auto resolved = nysor::resolve_conversions(nysor::load_graph_definition(pipeline_path), pipeline_registry);
        expect(resolved.conversions.size() == 1 && resolved.conversions.front().plan.steps.size() == 2,
          "conversion provenance was not retained");
        expect(resolved.graph.nodes.size() == 4, "dissolvers were not inserted into the resolved graph");
            expect(resolved.graph.nodes[2].block == "package.bridge.one" &&
             resolved.graph.nodes[3].block == "package.bridge.two",
              "resolved graph does not contain the selected Dissolver Blocks");
        expect(nysor::execute(nysor::lower_to_ir(
         nysor::build_graph(resolved.graph, pipeline_registry, pipeline_path))) == 9.0,
          "resolved dissolver graph did not execute");

    std::filesystem::remove_all(root);
    std::cout << "Nysor 0.9 Dissolver Resolution: PASS\n";
    return 0;
  } catch (const std::exception& error) {
    std::filesystem::remove_all(root);
    std::cerr << "FAILED: " << error.what() << '\n';
    return 1;
  }
}
