#pragma once

#include "Chelssy/Chess/Types.h"
#include <format>
#include <ostream>

namespace Chelssy::Chess {

inline void PrintTo(const Square &sqr, std::ostream *out) {
  if (sqr.isInvalid()) {
    *out << std::format("off-board({:x})", sqr.index());
    return;
  }

  const auto file = sqr.file();
  const auto rank = sqr.rank();
  *out << std::format("{}{}", static_cast<char>(file + 'a'),
                      static_cast<char>(rank + '1'));
}

} // namespace Chelssy::Chess
