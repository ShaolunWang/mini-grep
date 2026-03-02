#include <string>
class InputConfig {
private:
  static std::string pattern;
  // bitwise length initialization
  const std::size_t chunk_size{pattern.length()};

public:
  // singleton class
  InputConfig(const InputConfig &) = delete;
  InputConfig(InputConfig &&) = delete;
  InputConfig &operator=(const InputConfig &) = delete;
  InputConfig &operator=(InputConfig &&) = delete;
  explicit InputConfig(const std::string &pattern) {
    InputConfig::pattern = pattern;
  }
  static InputConfig *getInputConfig() {
    static InputConfig config(pattern);
    return &config;
  }
  size_t getChunkSize() const { return chunk_size; }
};
