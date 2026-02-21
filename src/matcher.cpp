#include "search/matcher.h"

std::size_t Re2Matcher::match(std::string_view chunk) const {
  std::size_t count = 0;
  re2::StringPiece input(chunk.data(), chunk.size());
  while (RE2::FindAndConsume(&input, m_regex)) {
    ++count;
  }
  return count;
}
