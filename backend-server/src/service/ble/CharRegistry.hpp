#ifndef CHAR_REGISTRY_H
#define CHAR_REGISTRY_H


#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <algorithm>
#include <mutex>          // use this if you’re on C++11/14
#include <shared_mutex>   // prefer this if you have C++17

struct CharMeta {
  std::string path;       // BlueZ object path
  std::string uuid;       // 128-bit UUID
  std::string name;       // friendly label
  int numberOfValues = 1; // how many values per notification
};

class CharRegistry {
public:
  // --- writers (unique lock) ---
  void upsert(const std::string& path,
              const std::string& uuid,
              const std::string& name,
              int numberOfValues)
  {
    std::unique_lock lock(m_); // exclusive
    CharMeta meta{path, uuid, name, numberOfValues};

    auto it = byPath_.find(path);
    if (it == byPath_.end()) {
      byPath_.emplace(path, meta);
      uuidToPaths_[uuid].push_back(path);
    } else {
      if (it->second.uuid != uuid) {
        removeFromUuidIndexUnlocked_(it->second.uuid, path);
        uuidToPaths_[uuid].push_back(path);
      }
      it->second = std::move(meta);
    }
  }

  void eraseByPath(const std::string& path) {
    std::unique_lock lock(m_);
    auto it = byPath_.find(path);
    if (it == byPath_.end()) return;
    removeFromUuidIndexUnlocked_(it->second.uuid, path);
    byPath_.erase(it);
  }

  void clear() {
    std::unique_lock lock(m_);
    byPath_.clear();
    uuidToPaths_.clear();
  }

  // --- readers (shared lock) ---
  std::optional<CharMeta> getByPath(const std::string& path) const {
    std::shared_lock lock(m_);
    if (auto it = byPath_.find(path); it != byPath_.end()) return it->second;
    return std::nullopt;
  }

  // Snapshot of all metas (safe to iterate without locks)
  std::vector<CharMeta> listAll() const {
    std::shared_lock lock(m_);
    std::vector<CharMeta> out;
    out.reserve(byPath_.size());
    for (const auto& kv : byPath_) out.push_back(kv.second);
    return out;
  }

  // Present UUIDs (unique)
  std::vector<std::string> listUuids() const {
    std::shared_lock lock(m_);
    std::vector<std::string> out;
    out.reserve(uuidToPaths_.size());
    for (const auto& kv : uuidToPaths_) out.push_back(kv.first);
    return out;
  }

  // Present names (may repeat if same name used for multiple chars)
  std::vector<std::string> listNames() const {
    std::shared_lock lock(m_);
    std::vector<std::string> out;
    out.reserve(byPath_.size());
    for (const auto& kv : byPath_) out.push_back(kv.second.name);
    return out;
  }

  // Paths for a given UUID
  std::vector<std::string> getPathsByUuid(const std::string& uuid) const {
    std::shared_lock lock(m_);
    if (auto it = uuidToPaths_.find(uuid); it != uuidToPaths_.end()) return it->second;
    return {};
  }

  // Convenience stats
  size_t size() const {
    std::shared_lock lock(m_);
    return byPath_.size();
  }
  bool containsPath(const std::string& path) const {
    std::shared_lock lock(m_);
    return byPath_.find(path) != byPath_.end();
  }

private:
  // Called only with m_ held (unique or shared where appropriate)
  void removeFromUuidIndexUnlocked_(const std::string& uuid, const std::string& path) {
    auto it = uuidToPaths_.find(uuid);
    if (it == uuidToPaths_.end()) return;
    auto& vec = it->second;
#if __cplusplus >= 202002L
    std::erase(vec, path);
#else
    vec.erase(std::remove(vec.begin(), vec.end(), path), vec.end());
#endif
    if (vec.empty()) uuidToPaths_.erase(it);
  }

  // Primary key: path → meta
  std::unordered_map<std::string, CharMeta> byPath_;
  // Secondary: uuid → list of paths
  std::unordered_map<std::string, std::vector<std::string>> uuidToPaths_;

  // Concurrency control
#if __cplusplus >= 201703L
  mutable std::shared_mutex m_;     // readers-writer lock (C++17)
#else
  mutable std::mutex m_;            // fallback (C++11/14: no shared lock)
  using shared_lock = std::unique_lock<std::mutex>;
#endif
};


#endif