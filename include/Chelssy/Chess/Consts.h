#pragma once

#include <cstdint>

namespace Chelssy::Chess {

constexpr uint8_t chessboardSize = 64;
constexpr uint8_t rankMax = 7;
constexpr uint8_t fileMax = 7;
constexpr uint8_t plyClockMax = 100;
constexpr uint16_t moveCounterMin = 1;
constexpr uint16_t moveCounterMax = 8850;

} // namespace Chelssy::Chess
