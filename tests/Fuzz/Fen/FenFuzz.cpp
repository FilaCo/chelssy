#include "Chelssy/Chess/Fen.h"
#include <array>
#include <vector>

using namespace Chelssy::Chess;

extern "C" auto LLVMFuzzerTestOneInput(const uint8_t *data, const size_t size)
    -> int {
  const std::string_view input{reinterpret_cast<const char *>(data), size};

  const auto parsed = Fen::parse(input);
  if (!parsed) {
    return 0;
  }

  // str_length_max is enough for any fen string.
  std::array<char, Fen::strLengthMax> buf{};
  const auto [ptr, ec] = parsed->toChars(buf.data(), buf.data() + buf.size());
  if (ec != std::errc{}) {
    __builtin_trap();
  }

  // round trip
  if (const auto reparsed = Fen::parse({buf.data(), ptr});
      !reparsed || *reparsed != *parsed) {
    __builtin_trap();
  }

  // value_too_large, if buffer is not enough
  const auto fullLen = static_cast<size_t>(ptr - buf.data());
  for (size_t n = 0; n < fullLen; ++n) {
    std::vector<char> small(n);
    if (const auto [smallPtr, smallEc] =
            parsed->toChars(small.data(), small.data() + n);
        smallEc != std::errc::value_too_large) {
      __builtin_trap();
    }
  }

  return 0;
}
