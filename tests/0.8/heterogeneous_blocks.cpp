#include "nysor/block_loader.hpp"

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

}  // namespace

int main() {
  const auto root = std::filesystem::temp_directory_path() / "nysor_08_blocks";
  std::filesystem::remove_all(root);
  try {
    const auto path = root / "core.time";
    std::filesystem::create_directories(path);
    write(path / "block.json", R"({"name":"Timer","version":"1.0.0","kind":"source","operation":"time","inputs":[],"outputs":[{"name":"delta_time","specification":"core.event"}]})");
    const auto block = nysor::load_block(path / "block.json");
    if (block.operation != "time" || block.output_ports[0].specification != "core.event") {
      throw std::runtime_error("heterogeneous block metadata was not loaded");
    }
    std::filesystem::remove_all(root);
    std::cout << "Heterogeneous Block loading: PASS\n";
    return 0;
  } catch (const std::exception& error) {
    std::filesystem::remove_all(root);
    std::cerr << "FAILED: " << error.what() << '\n';
    return 1;
  }
}
