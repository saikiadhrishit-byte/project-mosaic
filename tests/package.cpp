#include "nysor/package.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {

void expect(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void write(const std::filesystem::path& path, const std::string& content) {
  std::ofstream file(path);
  if (!file) throw std::runtime_error("Could not create test fixture");
  file << content;
}

std::filesystem::path make_package(const std::filesystem::path& root,
                                   const std::string& id,
                                   const std::string& version,
                                   const std::string& dependencies = "{}") {
  const auto path = root / id;
  std::filesystem::create_directories(path / "src");
  write(path / "package.json", "{\"id\":\"" + id + "\",\"name\":\"Add\",\"version\":\"" +
                               version + "\",\"dependencies\":" + dependencies + "}");
  if (id == "core.math.multiply") {
    write(path / "block.json", R"({"name":"multiply","version":"1.0.0","kind":"binary","operation":"multiply","inputs":[{"name":"left","type":"number"},{"name":"right","type":"number"}],"outputs":[{"name":"result","type":"number"}]})");
  } else {
    write(path / "block.json", R"({"name":"add","version":"1.0.0","kind":"binary","operation":"add","inputs":[{"name":"left","type":"number"},{"name":"right","type":"number"}],"outputs":[{"name":"result","type":"number"}]})");
  }
  return path;
}

}  // namespace

int main() {
  const auto root = std::filesystem::temp_directory_path() / "nysor_package_test";
  std::filesystem::remove_all(root);
  try {
    const auto add_path = make_package(root, "core.math.add", "1.0.0");
    const auto add = nysor::load_package(add_path);
    expect(add.id == "core.math.add" && add.block.operation == "add", "valid package failed");
    expect(std::filesystem::equivalent(add.block_manifest, add_path / "block.json"),
           "block manifest was not resolved relative to package");

    nysor::PackageRegistry registry;
    registry.install(add);
    expect(registry.contains("core.math.add") && registry.get("core.math.add").version == "1.0.0",
           "installed package was not registered");

    const auto missing_manifest = root / "missing";
    std::filesystem::create_directories(missing_manifest);
    write(missing_manifest / "block.json", "{}");
    try { nysor::load_package(missing_manifest); throw std::runtime_error("missing manifest accepted"); }
    catch (const std::runtime_error& error) { expect(std::string(error.what()).find("package.json missing") != std::string::npos, "wrong missing manifest error"); }

    const auto broken = root / "broken";
    std::filesystem::create_directories(broken);
    write(broken / "package.json", R"({"id":"broken","version":"1.0.0","block":"missing.json"})");
    try { nysor::load_package(broken); throw std::runtime_error("broken block accepted"); }
    catch (const std::runtime_error& error) { expect(std::string(error.what()).find("Referenced Block manifest does not exist") != std::string::npos, "wrong broken reference error"); }

    try { registry.install(nysor::load_package(add_path)); throw std::runtime_error("duplicate accepted"); }
    catch (const std::runtime_error& error) { expect(std::string(error.what()).find("Duplicate package ID") != std::string::npos, "wrong duplicate error"); }

    const auto missing_dependency = make_package(root, "game.controller", "1.0.0", R"({"physics.body":"^1.0.0"})");
    try { registry.install(nysor::load_package(missing_dependency)); throw std::runtime_error("missing dependency accepted"); }
    catch (const std::runtime_error& error) { expect(std::string(error.what()).find("Missing package: physics.body") != std::string::npos, "wrong missing dependency error"); }

    const auto incompatible = make_package(root, "game.incompatible", "1.0.0", R"({"core.math.add":"^2.0.0"})");
    try { registry.install(nysor::load_package(incompatible)); throw std::runtime_error("incompatible dependency accepted"); }
    catch (const std::runtime_error& error) { expect(std::string(error.what()).find("Version compatibility: FAIL") != std::string::npos, "wrong version error"); }

    std::filesystem::remove_all(root);
    std::cout << "Nysor 0.7 Package Loading and Registry: PASS\n";
    return 0;
  } catch (const std::exception& error) {
    std::filesystem::remove_all(root);
    std::cerr << "FAILED: " << error.what() << '\n';
    return 1;
  }
}