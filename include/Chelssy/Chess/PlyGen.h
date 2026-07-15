#pragma once

#include "Board.h"
#include "Chelssy/Chess/Consts.h"
#include "Chelssy/Chess/Types.h"
#include "Detail/AttackTable.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>

namespace Chelssy::Chess {

[[nodiscard]] constexpr auto isSquareAttacked(const Board &board,
                                              const Square sqr,
                                              const Color by) noexcept -> bool {
  using Detail::Attackers;
  const auto pawnAttacker =
      by == Color::White ? Attackers::WhitePawn : Attackers::BlackPawn;
  // Be careful with reordering this conditions - it can affect the
  // performance dramatically.
  return anyContactAttack(board, sqr, makePiece(by, PieceKind::Pawn),
                          pawnAttacker) ||
         anyContactAttack(board, sqr, makePiece(by, PieceKind::Knight),
                          Attackers::Knight) ||
         anyContactAttack(board, sqr, makePiece(by, PieceKind::King),
                          Attackers::King) ||
         anySliderAttack(board, sqr, makePiece(by, PieceKind::Bishop),
                         Attackers::Bishop) ||
         anySliderAttack(board, sqr, makePiece(by, PieceKind::Rook),
                         Attackers::Rook) ||
         anySliderAttack(board, sqr, makePiece(by, PieceKind::Queen),
                         Attackers::Queen);
}

constexpr size_t pliesPerPositionMax = 256;

enum class GenKind : uint8_t {
  /// Captures, en passant, and promotions
  ChangesMaterial,
  All,
};

namespace Detail {

/// Number of promotion kinds (knight, bishop, rook, queen).
constexpr uint8_t promoKindsCount = 4;

constexpr void emitPromos(const Square from, const Square to,
                          const bool capture, const std::span<Ply> out,
                          size_t &count) noexcept {
  const auto base = std::to_underlying(capture ? Ply::Flag::KnightPromoCapture
                                               : Ply::Flag::KnightPromo);
  for (uint8_t kind = 0; kind < promoKindsCount; ++kind) {
    out[count++] = Ply(from, to, static_cast<Ply::Flag>(base + kind));
  }
}

template <GenKind Kind>
constexpr void genPawnPlies(const Board &board, const std::span<Ply> out,
                            size_t &count) noexcept {
  const auto us = board.sideToMove();
  const auto them = flip(us);
  const uint8_t startRank = relativeRank(us, 1);
  const uint8_t prePromoRank = relativeRank(us, rankMax - 1);
  const auto epSqr = board.enPassantTargetSquare();

  for (const auto from : board.squares(makePiece(us, PieceKind::Pawn))) {
    const auto promo = from.rank() == prePromoRank;

    for (const auto off : Square::pawnAttackOffsets[std::to_underlying(us)]) {
      const auto to = from.shifted(off);
      if (to.isInvalid()) {
        continue;
      }
      if (to == epSqr) {
        out[count++] = Ply(from, to, Ply::Flag::EnPassantCapture);
        continue;
      }
      if (!hasColor(board.pieceAt(to), them)) {
        continue;
      }
      if (promo) {
        emitPromos(from, to, /*capture=*/true, out, count);
      } else {
        out[count++] = Ply(from, to, Ply::Flag::Capture);
      }
    }

    const auto push = from.shiftedForwards(us);
    assert(push.isValid() && "pawn on its promotion rank");
    if (!board.isEmpty(push)) {
      continue;
    }
    if (promo) {
      emitPromos(from, push, /*capture=*/false, out, count);
      continue;
    }
    if constexpr (Kind == GenKind::All) {
      out[count++] = Ply(from, push, Ply::Flag::Quiet);
      if (from.rank() == startRank) {
        const auto jump = push.shiftedForwards(us);
        if (board.isEmpty(jump)) {
          out[count++] = Ply(from, jump, Ply::Flag::DoublePawnPush);
        }
      }
    }
  }
}

/// Generates plies for pieces with single-step attacks (knight, king).
template <GenKind Kind>
constexpr void genContactPlies(const Board &board, const PieceKind kind,
                               const std::span<const int8_t> offsets,
                               const std::span<Ply> out,
                               size_t &count) noexcept {
  const auto us = board.sideToMove();
  const auto them = flip(us);
  for (const auto from : board.squares(makePiece(us, kind))) {
    for (const auto off : offsets) {
      const auto to = from.shifted(off);
      if (to.isInvalid()) {
        continue;
      }
      const auto target = board.pieceAt(to);
      if (isNone(target)) {
        if constexpr (Kind == GenKind::All) {
          out[count++] = Ply(from, to, Ply::Flag::Quiet);
        }
      } else if (hasColor(target, them)) {
        out[count++] = Ply(from, to, Ply::Flag::Capture);
      }
    }
  }
}

/// Generates plies for ray pieces (bishop, rook, queen).
template <GenKind Kind>
constexpr void genSliderPlies(const Board &board, const PieceKind kind,
                              const std::span<const int8_t> dirs,
                              const std::span<Ply> out,
                              size_t &count) noexcept {
  const auto us = board.sideToMove();
  const auto them = flip(us);
  for (const auto from : board.squares(makePiece(us, kind))) {
    for (const auto dir : dirs) {
      for (auto to = from.shifted(dir); to.isValid(); to = to.shifted(dir)) {
        const auto target = board.pieceAt(to);
        if (isNone(target)) {
          if constexpr (Kind == GenKind::All) {
            out[count++] = Ply(from, to, Ply::Flag::Quiet);
          }
          continue;
        }
        if (hasColor(target, them)) {
          out[count++] = Ply(from, to, Ply::Flag::Capture);
        }
        break;
      }
    }
  }
}

constexpr void genCastlePlies(const Board &board, const std::span<Ply> out,
                              size_t &count) noexcept {
  const auto us = board.sideToMove();
  const auto them = flip(us);
  const auto kingRight = kingCastleRight(us);
  const auto queenRight = queenCastleRight(us);
  const auto rights = board.castlingRights();
  if (!hasAny(rights, kingRight | queenRight)) {
    return;
  }

  // A castle out of, through, or into check is illegal, and the search's
  // post-doPly legality filter only sees the king's destination - so the
  // "out of" and "through" parts must be rejected here. Checking the
  // destination too makes emitted castles fully legal for free.
  const auto kingFrom = board.kingSquare(us);
  if (isSquareAttacked(board, kingFrom, them)) {
    return;
  }

  const auto rank = relativeRank(us, 0);
  assert(kingFrom == Square(4, rank) && "castling rights with a moved king");

  if (hasAny(rights, kingRight)) {
    const Square transit{5, rank};
    const Square landing{6, rank};
    if (board.isEmpty(transit) && board.isEmpty(landing) &&
        !isSquareAttacked(board, transit, them) &&
        !isSquareAttacked(board, landing, them)) {
      out[count++] = Ply(kingFrom, landing, Ply::Flag::KingCastle);
    }
  }
  if (hasAny(rights, queenRight)) {
    // The b-file square only has to be empty (the rook passes through
    // it); attacks matter on the king's path alone.
    const Square rookGap{1, rank};
    const Square landing{2, rank};
    const Square transit{3, rank};
    if (board.isEmpty(rookGap) && board.isEmpty(landing) &&
        board.isEmpty(transit) && !isSquareAttacked(board, transit, them) &&
        !isSquareAttacked(board, landing, them)) {
      out[count++] = Ply(kingFrom, landing, Ply::Flag::QueenCastle);
    }
  }
}

} // namespace Detail

/// Writes pseudo-legal plies for the side to move into `out` and returns
/// the filled prefix.
///
/// Pseudo-legal means the mover's king may be left in check; the caller
/// filters that out with doPly + isSquareAttacked + undoPly. Castles are
/// the exception: they are emitted fully legal, see genCastlePlies.
///
/// @pre out.size() >= pliesPerPositionMax
template <GenKind Kind>
[[nodiscard]]
constexpr auto genPlies(const Board &board, std::span<Ply> out) noexcept
    -> std::span<Ply> {
  assert(out.size() >= pliesPerPositionMax && "`out` buffer is too small");
  size_t count = 0;
  Detail::genPawnPlies<Kind>(board, out, count);
  Detail::genContactPlies<Kind>(board, PieceKind::Knight, Square::knightOffsets,
                                out, count);
  Detail::genSliderPlies<Kind>(board, PieceKind::Bishop, Square::bishopOffsets,
                               out, count);
  Detail::genSliderPlies<Kind>(board, PieceKind::Rook, Square::rookOffsets, out,
                               count);
  // Queen rays are the union of bishop and rook rays.
  Detail::genSliderPlies<Kind>(board, PieceKind::Queen, Square::bishopOffsets,
                               out, count);
  Detail::genSliderPlies<Kind>(board, PieceKind::Queen, Square::rookOffsets,
                               out, count);
  Detail::genContactPlies<Kind>(board, PieceKind::King, Square::kingOffsets,
                                out, count);
  if constexpr (Kind == GenKind::All) {
    Detail::genCastlePlies(board, out, count);
  }
  return out.first(count);
}

} // namespace Chelssy::Chess
