//
// Created by Dmitrii Chizha on 01.07.2026.
//

#pragma once

#include <cassert>
#include <compare>
#include <cstdint>
#include <utility>

namespace chelssy::chess {

/// Side of play. Also used to tag piece ownership.
enum class color : uint8_t {
  white = 0,
  black,
};

[[nodiscard]] constexpr auto opposite(const color c) noexcept -> color {
  return static_cast<color>(std::to_underlying(c) ^ 1);
}

/// Piece kind combined with its color.
///
/// White pieces are listed before black ones, in the same kind order
/// (pawn..king). Code may rely on this ordering.
enum class piece : uint8_t {
  /// Represents empty square.
  none = 0,
  white_pawn,
  white_knight,
  white_bishop,
  white_rook,
  white_queen,
  white_king,
  black_pawn,
  black_knight,
  black_bishop,
  black_rook,
  black_queen,
  black_king,
};

/// Castling rights encoded as a single byte.
/// To check if castling is available, do:
/// `has_any(castling, castling_rights::white_queen)`.
///
/// For multiple check use a mask, e.g.:
/// `has_any(castling, castling_rights::black_king |
/// castling_rights::black_queen).
enum class castling_rights : uint8_t {
  /// No castling is available.
  none = 0,
  /// King castle is available for white side.
  white_king = 1 << 0,
  /// Queen castle is available for white side.
  white_queen = 1 << 1,
  /// King castle is available for black side.
  black_king = 1 << 2,
  /// Queen castle is available for black side.
  black_queen = 1 << 3,
  all = white_king | white_queen | black_king | black_queen,
};

[[nodiscard]] constexpr auto operator&(const castling_rights lhs,
                                       const castling_rights rhs) noexcept
    -> castling_rights {
  return static_cast<castling_rights>(std::to_underlying(lhs) &
                                      std::to_underlying(rhs));
}

[[nodiscard]] constexpr auto operator|(const castling_rights lhs,
                                       const castling_rights rhs) noexcept
    -> castling_rights {
  return static_cast<castling_rights>(std::to_underlying(lhs) |
                                      std::to_underlying(rhs));
}

/// Complement *within the valid flag set*: unused bits of the byte are
/// never set, so `~x` is always a valid castling_rights value.
[[nodiscard]] constexpr auto operator~(const castling_rights v) noexcept
    -> castling_rights {
  return static_cast<castling_rights>(~std::to_underlying(v) &
                                      std::to_underlying(castling_rights::all));
}

constexpr auto operator&=(castling_rights &lhs,
                          const castling_rights rhs) noexcept
    -> castling_rights & {
  lhs = lhs & rhs;
  return lhs;
}

constexpr auto operator|=(castling_rights &lhs,
                          const castling_rights rhs) noexcept
    -> castling_rights & {
  lhs = lhs | rhs;
  return lhs;
}

constexpr auto has_any(const castling_rights val,
                       const castling_rights mask) noexcept -> bool {
  return (val & mask) != castling_rights::none;
}

/// A board square in 0x88 encoding.
///
/// The square index is a single byte laid out as `0rrr_0fff`:
/// - bits 0-2  file (0=a .. 7=h)
/// - bit  3    file-overflow flag
/// - bits 4-6  rank (0=rank 1 .. 7=rank 8)
/// - bit  7    rank-overflow flag
///
/// This is the defining property of 0x88: a square is on the board if
/// none of the overflow bits are set, i.e. `(index & 0x88) == 0`.
struct square {
  /// @pre `rank < 8 && file < 8`
  constexpr square(const uint8_t rank, const uint8_t file) noexcept
      : square(rank << rank_shift | file) {
    assert(rank < 8 && file < 8 && "invalid square");
  }

  /// Sentinel "no square" value (e.g. no en-passant target).
  ///
  /// @note This is *one* particular invalid square. Invalid squares are not
  /// unique, so `sqr == square::none()` is NOT a validity check - a square
  /// produced by `shifted()` may be invalid yet differ from `none()`. Use
  /// `is_valid()` to test validity.
  [[nodiscard]] static constexpr auto none() noexcept -> square {
    return square(offboard_mask);
  }

  /// @note Allows to make invalid squares. Check the return value with
  /// `is_valid()`.
  [[nodiscard]] static constexpr auto from_index(const uint8_t index) noexcept
      -> square {
    return square(index);
  }

  [[nodiscard]] constexpr auto is_valid() const noexcept -> bool {
    return (inner_ & offboard_mask) == 0;
  }

  /// @pre `is_valid()`; for off-board squares the result is meaningless.
  [[nodiscard]] constexpr auto rank() const noexcept -> uint8_t {
    return inner_ >> rank_shift;
  }
  /// @pre `is_valid()`; for off-board squares the result is meaningless.
  [[nodiscard]] constexpr auto file() const noexcept -> uint8_t {
    return inner_ & file_mask;
  }

  [[nodiscard]] constexpr auto operator<=>(const square &other) const noexcept
      -> std::strong_ordering = default;

  /// Raw 0x88 index. `0..0x77` for valid squares; mailbox arrays must be
  /// 128 entries to be indexable by any representable square.
  [[nodiscard]] constexpr auto index() const noexcept -> uint8_t {
    return inner_;
  }

  /// Returns the square displaced by a 0x88 offset: 0x10 is one rank up,
  /// 0x01 one file right (e.g. north-east = 0x11, a knight jump = +-0x21,
  /// +-0x1F, +-0x12, +-0x0E).
  ///
  /// @note Allows to make invalid squares. Check the return value with
  /// `is_valid()`.
  [[nodiscard]] constexpr auto shifted(const int8_t offset) const noexcept
      -> square {
    return from_index(inner_ + offset);
  }

private:
  uint8_t inner_;

  static constexpr uint8_t rank_shift = 4;
  static constexpr uint8_t file_mask = 7;
  static constexpr uint8_t offboard_mask = 0x88;

  constexpr explicit square(const uint8_t inner) noexcept : inner_(inner) {}
};

struct ply {};

} // namespace chelssy::chess
