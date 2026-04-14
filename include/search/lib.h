#pragma once

#include "policies.h"
#include <cstddef>
#include <thread>
#include <unistd.h>

template <ExecutorPolicy Policy> class Engine {
public:
  explicit Engine(std::unique_ptr<Policy> policy)
      : m_policy{std::move(policy)} {}
  size_t run() {
    std::jthread producer([this] { this->read_io(); });
    producer.join();
    return m_policy->wait();
  };
  void setFilePath(const std::string &s) { m_filePath = s; }

private:
  void read_io();

  std::unique_ptr<Policy> m_policy;
  std::string m_filePath;
};
