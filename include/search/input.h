#pragma once
#include <cstddef>
#include <string>

class InputConfig {
public:
  InputConfig(const InputConfig &) = delete;
  InputConfig(InputConfig &&) = delete;
  InputConfig &operator=(const InputConfig &) = delete;
  InputConfig &operator=(InputConfig &&) = delete;

  static void init(const std::string &pattern) {
    m_pattern = pattern;
    m_initialized = true;
  }

  static InputConfig *getInputConfig() {
    static InputConfig config;
    return &config;
  }

  static size_t getChunkSize() { return m_pattern.length(); }
  static const std::string &getPattern() { return m_pattern; }

private:
  InputConfig() = default;
  inline static std::string m_pattern;
  inline static bool m_initialized{false};
};
;
