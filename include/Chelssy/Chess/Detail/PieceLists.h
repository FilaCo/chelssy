#pragma once

#include "Chelssy/Chess/Consts.h"
#include "Chelssy/Chess/Types.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <span>
#include <utility>

namespace Chelssy::Chess::Detail {

struct PieceLists {
  [[nodiscard]] constexpr auto count(const Piece piece) const noexcept
      -> uint8_t {
    return count(getColor(piece), getKind(piece));
  }

  [[nodiscard]] constexpr auto count(const Color color,
                                     const PieceKind kind) const noexcept
      -> uint8_t {
    const auto colorIdx = std::to_underlying(color);
    const auto kindIdx = std::to_underlying(kind);

    return counts_[colorIdx][kindIdx];
  }

  [[nodiscard]] constexpr auto squares(Color color,
                                       PieceKind kind) const noexcept
      -> std::span<const Square> {
    const auto colorIdx = std::to_underlying(color);
    const auto kindIdx = std::to_underlying(kind);
    const auto cnt = count(color, kind);
    return std::span{sqrs_[colorIdx][kindIdx]}.first(cnt);
  }

  /// @pre `sqr.isValid() == true`
  constexpr void add(const Piece piece, const Square sqr) noexcept {
    assert(sqr.isValid());
    const auto color = std::to_underlying(getColor(piece));
    const auto kind = std::to_underlying(getKind(piece));

    auto &count = counts_[color][kind];
    sqrs_[color][kind][count] = sqr;
    const auto sqr8x8 = sqr.to8x8();
    indexBoards_[sqr8x8] = count;
    ++count;
  }

private:
  // 2 initial + 8 promoted pawns.
  static constexpr uint8_t pieceListCapacity = 10;

  std::array<std::array<std::array<Square, pieceListCapacity>, pieceKindsCount>,
             colorsCount>
      sqrs_{};
  std::array<std::array<uint8_t, pieceKindsCount>, colorsCount> counts_{};
  std::array<uint8_t, chessboardSize> indexBoards_{};
};

} // namespace Chelssy::Chess::Detail
