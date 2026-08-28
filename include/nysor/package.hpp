#pragma once

#include "nysor/block_loader.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>

namespace nysor {

struct PackageDependency {
  std::string id;
  std::string version_requirement;
};

struct PackageDefinition {
  std::string id;
  std::string name;
  std::string version;
  std::string author;
  std::string license;
  std::filesystem::path root;
  std::filesystem::path manifest;
  std::filesystem::path block_manifest;
  BlockDefinition block;
  std::unordered_map<std::string, PackageDependency> dependencies;
};

PackageDefinition load_package(const std::filesystem::path& path);
bool version_satisfies(const std::string& version,
                       const std::string& requirement);

class PackageRegistry {
public:
  void install(PackageDefinition package);
  bool contains(const std::string& package_id) const;
  const PackageDefinition& get(const std::string& package_id) const;

private:
  std::unordered_map<std::string, PackageDefinition> packages_;
};

}  // namespace nysor