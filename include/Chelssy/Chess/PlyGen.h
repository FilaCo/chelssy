#pragma once

#include "Board.h"
#include "Chelssy/Chess/Types.h"
#include "Detail/AttackTable.h"

namespace Chelssy::Chess {

struct PlyGen {
  [[nodiscard]] static constexpr auto isSquareAttacked(const Board &board,
                                                       const Square sqr,
                                                       const Color by) noexcept
      -> bool {
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
};

} // namespace Chelssy::Chess