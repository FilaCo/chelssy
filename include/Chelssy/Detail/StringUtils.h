#pragma once

namespace Chelssy::Detail {

constexpr auto putChr(char *&first, const char *last, const char chr) noexcept
    -> bool {
  if (last == first) {
    return false;
  }

  *first++ = chr;
  return true;
}

} // namespace Chelssy::Detail
