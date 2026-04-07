#pragma once
#include "re2/re2.h"

class Re2Matcher {
public:
  explicit Re2Matcher(const std::string &pattern)
      : m_regex(std::move(pattern)) {}
  std::size_t match(std::string_view chunk) const;

private:
  re2::RE2 m_regex;
};
