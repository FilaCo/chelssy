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
    return counts_[getColorIdx(piece)][getKindIdx(piece)];
  }

  [[nodiscard]] constexpr auto squares(const Piece piece) const noexcept
      -> std::span<const Square> {
    return std::span{sqrs_[getColorIdx(piece)][getKindIdx(piece)]}.first(
        count(piece));
  }

  /// @pre `sqr.isValid() == true`
  constexpr void add(const Piece piece, const Square sqr) noexcept {
    assert(sqr.isValid() && "invalid piece position");
    const auto color = getColorIdx(piece);
    const auto kind = getKindIdx(piece);

    auto &cnt = counts_[color][kind];
    assert(cnt < pieceListCapacity && "piece count exceeded");
    sqrs_[color][kind][cnt] = sqr;
    const auto sqr8x8 = sqr.to8x8();
    indexBoard_[sqr8x8] = cnt;
    ++cnt;
  }

  /// @pre `from.isValid() && to.isValid()`
  /// @pre piece is on the board.
  /// @note in case of capturing remove the captured piece firstly, then move
  /// the capturer.
  constexpr void move(const Piece piece, const Square from,
                      const Square to) noexcept {
    assert(from.isValid() && "`from` square is invalid");
    assert(to.isValid() && "`to` square is invalid");

    const auto idx = indexBoard_[from.to8x8()];
    const auto colorIdx = getColorIdx(piece);
    const auto kindIdx = getKindIdx(piece);
    assert(sqrs_[colorIdx][kindIdx][idx] == from &&
           "piece is not on the board");

    indexBoard_[to.to8x8()] = idx;
    sqrs_[colorIdx][kindIdx][idx] = to;
  }

  /// @pre `sqr.isValid()` and piece is on the board.
  constexpr void remove(const Piece piece, const Square sqr) noexcept {
    assert(sqr.isValid() && "sqr is invalid");

    const auto idx = indexBoard_[sqr.to8x8()];
    const auto colorIdx = getColorIdx(piece);
    const auto kindIdx = getKindIdx(piece);
    assert(sqrs_[colorIdx][kindIdx][idx] == sqr && "piece is not on the board");

    auto &cnt = counts_[colorIdx][kindIdx];
    assert(cnt > 0 && "can't remove piece that doesn't exist");
    --cnt;

    auto &lastSqr = sqrs_[colorIdx][kindIdx][cnt];
    indexBoard_[lastSqr.to8x8()] = idx;
    std::swap(sqrs_[colorIdx][kindIdx][idx], lastSqr);
  }

private:
  // 2 initial + 8 promoted pawns.
  static constexpr uint8_t pieceListCapacity = 10;

  std::array<std::array<std::array<Square, pieceListCapacity>, pieceKindsCount>,
             colorsCount>
      sqrs_{};
  std::array<std::array<uint8_t, pieceKindsCount>, colorsCount> counts_{};
  std::array<uint8_t, chessboardSize> indexBoard_{};

  [[nodiscard]] static constexpr auto getColorIdx(const Piece piece) noexcept
      -> uint8_t {
    return std::to_underlying(getColor(piece));
  }

  [[nodiscard]] static constexpr auto getKindIdx(const Piece piece) noexcept
      -> uint8_t {
    return std::to_underlying(getKind(piece));
  }
};

} // namespace Chelssy::Chess::Detail
