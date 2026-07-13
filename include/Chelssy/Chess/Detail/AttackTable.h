#pragma once

#include "Chelssy/Chess/Board.h"
#include "Chelssy/Chess/Types.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace Chelssy::Chess::Detail {

enum class Attackers : uint8_t {
  None = 0,
  WhitePawn = 1 << 0,
  BlackPawn = 1 << 1,
  Knight = 1 << 2,
  Bishop = 1 << 3,
  Rook = 1 << 4,
  Queen = 1 << 5,
  King = 1 << 6
};

[[nodiscard]] constexpr auto operator&(const Attackers lhs,
                                       const Attackers rhs) noexcept
    -> Attackers {
  return static_cast<Attackers>(std::to_underlying(lhs) &
                                std::to_underlying(rhs));
}

[[nodiscard]] constexpr auto operator|(const Attackers lhs,
                                       const Attackers rhs) noexcept
    -> Attackers {
  return static_cast<Attackers>(std::to_underlying(lhs) |
                                std::to_underlying(rhs));
}

constexpr auto operator|=(Attackers &lhs, const Attackers rhs) noexcept
    -> Attackers & {
  lhs = lhs | rhs;
  return lhs;
}

constexpr auto hasAny(const Attackers val, const Attackers mask) noexcept
    -> bool {
  return (val & mask) != Attackers::None;
}

struct AttackEntry {
  Attackers attackers;
  int8_t delta;
};

static constexpr uint8_t attackTableCenter = 0x77;
static constexpr uint8_t attackTableSize = 240;

using AttackTable = std::array<AttackEntry, attackTableSize>;

inline constexpr auto attackTable = []() noexcept -> AttackTable {
  AttackTable table{};

  const auto stamp = [&table](const int8_t dir, const Attackers attackers,
                              const uint8_t maxDist) noexcept -> void {
    for (uint8_t dist = 1; dist <= maxDist; ++dist) {
      auto &entry =
          table[static_cast<size_t>(attackTableCenter + (dir * dist))];
      entry.attackers |= attackers;
      entry.delta = dir;
    }
  };

  for (const auto dir : Square::rookOffsets) {
    stamp(dir, Attackers::Rook | Attackers::Queen, 7);
  }
  for (const auto dir : Square::bishopOffsets) {
    stamp(dir, Attackers::Bishop | Attackers::Queen, 7);
  }
  for (const auto dir : Square::kingOffsets) {
    stamp(dir, Attackers::King, 1);
  }

  for (const auto off : Square::knightOffsets) {
    table[static_cast<size_t>(attackTableCenter + off)].attackers |=
        Attackers::Knight;
  }

  for (const auto off :
       Square::pawnAttackOffsets[std::to_underlying(Color::White)]) {
    table[static_cast<size_t>(attackTableCenter + off)].attackers |=
        Attackers::WhitePawn;
  }
  for (const auto off :
       Square::pawnAttackOffsets[std::to_underlying(Color::Black)]) {
    table[static_cast<size_t>(attackTableCenter + off)].attackers |=
        Attackers::BlackPawn;
  }

  return table;
}();

static_assert(
    attackTable[attackTableCenter + (static_cast<size_t>(3 * Square::north))]
        .attackers == (Attackers::Rook | Attackers::Queen));
static_assert(
    attackTable[attackTableCenter + (static_cast<size_t>(3 * Square::north))]
        .delta == Square::north);
static_assert(attackTable[attackTableCenter + Square::northEast].attackers ==
              (Attackers::WhitePawn | Attackers::Bishop | Attackers::Queen |
               Attackers::King));
static_assert(attackTable[attackTableCenter + Square::northNorthEast]
                  .attackers == Attackers::Knight);
static_assert(attackTable[attackTableCenter + Square::northNorthEast].delta ==
              0);
// Black pawns attack downwards; the white-pawn bit must not leak here.
static_assert(attackTable[attackTableCenter + Square::southEast].attackers ==
              (Attackers::BlackPawn | Attackers::Bishop | Attackers::Queen |
               Attackers::King));
// Negative direction at the table's far edge (index 0).
static_assert(attackTable[attackTableCenter + (7 * Square::southWest)]
                  .attackers == (Attackers::Bishop | Attackers::Queen));
static_assert(attackTable[attackTableCenter + (7 * Square::southWest)].delta ==
              Square::southWest);
// A == B, never hit
static_assert(attackTable[attackTableCenter].attackers == Attackers::None);

[[nodiscard]] constexpr auto attackIndex(const Square from,
                                         const Square to) noexcept -> uint8_t {
  return attackTableCenter + to.index() - from.index();
}

[[nodiscard]] constexpr auto
anyContactAttack(const Board &board, const Square sqr, const Piece piece,
                 const Attackers attackers) noexcept -> bool {
  const auto squares = board.squares(piece);
  return std::ranges::any_of(
      squares, [sqr, attackers](auto &&from) noexcept -> bool {
        return hasAny(attackTable[attackIndex(from, sqr)].attackers, attackers);
      });
}

[[nodiscard]] constexpr auto
anySliderAttack(const Board &board, const Square sqr, const Piece piece,
                const Attackers attackers) noexcept -> bool {
  const auto squares = board.squares(piece);
  return std::ranges::any_of(
      squares, [&board, sqr, attackers](auto &&from) noexcept -> bool {
        const auto &entry = attackTable[attackIndex(from, sqr)];
        if (!hasAny(entry.attackers, attackers)) {
          return false;
        }

        // Walk from attacker to target
        auto sq =
            Square::fromIndex(static_cast<uint8_t>(from.index() + entry.delta));
        while (sq != sqr && board.pieceAt(sq) == Piece::None) {
          sq =
              Square::fromIndex(static_cast<uint8_t>(sq.index() + entry.delta));
        }
        return sq == sqr;
      });
}

} // namespace Chelssy::Chess::Detail