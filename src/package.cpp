#include "nysor/package.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace nysor {

namespace {

using Json = nlohmann::json;

std::runtime_error package_error(const std::filesystem::path& path,
                                 const std::string& problem) {
  return std::runtime_error("Package Error\n\nPath:\n" + path.string() +
                            "\n\nProblem:\n" + problem);
}

Json read_package_json(const std::filesystem::path& path) {
  std::ifstream file(path);
  if (!file) throw package_error(path, "package.json missing");
  try {
    return Json::parse(file);
  } catch (const Json::parse_error& error) {
    throw package_error(path, "Invalid JSON: " + std::string(error.what()));
  }
}

std::string required_string(const Json& json,
                            const std::filesystem::path& path,
                            const char* field) {
  if (!json.contains(field) || !json[field].is_string() ||
      json[field].get<std::string>().empty()) {
    throw package_error(path, "Required non-empty string is missing: " +
                                 std::string(field));
  }
  return json[field].get<std::string>();
}

struct Semver {
  int major = 0;
  int minor = 0;
  int patch = 0;
};

bool parse_version(const std::string& text, Semver& version) {
  char first_separator = 0;
  char second_separator = 0;
  std::istringstream stream(text);
  return (stream >> version.major >> first_separator >> version.minor >>
          second_separator >> version.patch) &&
         first_separator == '.' && second_separator == '.' && stream.eof();
}

bool version_at_least(const Semver& left, const Semver& right) {
  if (left.major != right.major) return left.major > right.major;
  if (left.minor != right.minor) return left.minor > right.minor;
  return left.patch >= right.patch;
}

bool same_version(const Semver& left, const Semver& right) {
  return left.major == right.major && left.minor == right.minor &&
         left.patch == right.patch;
}

}  // namespace

bool version_satisfies(const std::string& version,
                       const std::string& requirement) {
  Semver installed;
  Semver required;
  if (!parse_version(version, installed)) return false;
  if (requirement.size() > 1 && requirement[0] == '^') {
    if (!parse_version(requirement.substr(1), required)) return false;
    return installed.major == required.major && version_at_least(installed, required);
  }
  return parse_version(requirement, required) && same_version(installed, required);
}

PackageDefinition load_package(const std::filesystem::path& path) {
  const auto root = std::filesystem::absolute(path);
  if (!std::filesystem::is_directory(root)) {
    throw package_error(root, "Package directory does not exist");
  }
  const auto manifest = root / "package.json";
  const auto json = read_package_json(manifest);
  PackageDefinition package;
  package.id = required_string(json, manifest, "id");
  package.name = json.contains("name") ? required_string(json, manifest, "name") : package.id;
  package.version = required_string(json, manifest, "version");
  package.author = json.contains("author") ? required_string(json, manifest, "author") : "";
  package.license = json.contains("license") ? required_string(json, manifest, "license") : "";
  package.root = root;
  package.manifest = manifest;
  const auto block_reference = json.contains("block")
                                   ? required_string(json, manifest, "block")
                                   : "block.json";
  package.block_manifest = root / block_reference;
  if (!std::filesystem::is_regular_file(package.block_manifest)) {
    throw package_error(manifest, "Referenced Block manifest does not exist: " +
                                  block_reference);
  }
  package.block = load_block(package.block_manifest);

  if (json.contains("dependencies")) {
    if (!json["dependencies"].is_object()) {
      throw package_error(manifest, "dependencies must be an object");
    }
    for (const auto& [id, requirement] : json["dependencies"].items()) {
      if (!requirement.is_string() || requirement.get<std::string>().empty()) {
        throw package_error(manifest, "Dependency requirement must be a non-empty string: " + id);
      }
      package.dependencies.emplace(id, PackageDependency{id, requirement.get<std::string>()});
    }
  }
  return package;
}

void PackageRegistry::install(PackageDefinition package) {
  if (package.id.empty()) throw std::runtime_error("Package Error\n\nProblem:\nPackage ID is empty");
  if (packages_.contains(package.id)) {
    throw std::runtime_error("Package Error\n\nProblem:\nDuplicate package ID: " + package.id);
  }
  for (const auto& [id, dependency] : package.dependencies) {
    const auto installed = packages_.find(id);
    if (installed == packages_.end()) {
      throw std::runtime_error("Dependency resolution: FAIL\nMissing package: " + id);
    }
    if (!version_satisfies(installed->second.version, dependency.version_requirement)) {
      throw std::runtime_error("Version compatibility: FAIL\nPackage: " + id);
    }
  }
  const auto package_id = package.id;
  const auto conversion = package.block.conversion;
  packages_.emplace(package_id, std::move(package));
  if (conversion) {
    dissolvers_[conversion->from].push_back(package_id);
    auto& candidates = dissolvers_[conversion->from];
    std::sort(candidates.begin(), candidates.end());
  }
}

void PackageRegistry::discover(const std::filesystem::path& root) {
  if (!std::filesystem::is_directory(root)) {
    throw std::runtime_error("Package discovery root does not exist: " + root.string());
  }
  for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
    if (entry.is_regular_file() && entry.path().filename() == "package.json") {
      install(load_package(entry.path().parent_path()));
    }
  }
}

bool PackageRegistry::contains(const std::string& package_id) const {
  return packages_.contains(package_id);
}

const PackageDefinition& PackageRegistry::get(const std::string& package_id) const {
  const auto package = packages_.find(package_id);
  if (package == packages_.end()) {
    throw std::out_of_range("Package is not installed: " + package_id);
  }
  return package->second;
}

std::vector<std::string> PackageRegistry::dissolvers_from(
    const std::string& specification) const {
  const auto found = dissolvers_.find(specification);
  return found == dissolvers_.end() ? std::vector<std::string>{} : found->second;
}

}  // namespace nysor