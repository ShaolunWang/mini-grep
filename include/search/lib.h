#pragma once

#include "policies.h"
#include <cstddef>
#include <thread>
#include <unistd.h>

template <ExecutorPolicy Policy> class Engine {
public:
  explicit Engine(Policy policy) : m_policy{policy} {}
  size_t run() {
    std::jthread producer([this] { read_io(); });
    return m_policy.wait();
  };

private:
  void read_io();

  Policy m_policy;
};
