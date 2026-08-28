#include "nysor/package.hpp"

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
                                   const std::string& output_specification) {
  const auto path = root / id;
  std::filesystem::create_directories(path);
  write(path / "package.json", "{\"id\":\"" + id + "\",\"version\":\"1.0.0\"}");
  write(path / "block.json", "{\"name\":\"" + id + "\",\"version\":\"1.0.0\",\"kind\":\"unary\",\"operation\":\"event\",\"inputs\":[{\"name\":\"input\",\"specification\":\"core.event\"}],\"outputs\":[{\"name\":\"output\",\"specification\":\"" + output_specification + "\"}]}");
  return path;
}

}  // namespace

int main() {
  const auto root = std::filesystem::temp_directory_path() / "nysor_08_independent";
  std::filesystem::remove_all(root);
  try {
    nysor::PackageRegistry registry;
    const auto timer = nysor::load_package(make_package(root, "demo.timer", "core.event"));
    const auto counter = nysor::load_package(make_package(root, "demo.counter", "core.event"));
    registry.install(timer);
    registry.install(counter);
    if (timer.dependencies.size() != 0 || counter.dependencies.size() != 0 ||
        !nysor::validate_connection(timer.block, "output", counter.block, "input").compatible) {
      throw std::runtime_error("independent packages did not connect through core.event");
    }
    std::filesystem::remove_all(root);
    std::cout << "Independent package composition: PASS\n";
    return 0;
  } catch (const std::exception& error) {
    std::filesystem::remove_all(root);
    std::cerr << "FAILED: " << error.what() << '\n';
    return 1;
  }
}
