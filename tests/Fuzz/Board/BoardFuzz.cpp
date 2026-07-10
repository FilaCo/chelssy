#include "Chelssy/Chess/Board.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>

using namespace Chelssy::Chess;

namespace {

void checkMailboxMatchesPieceLists(const Board &board) {
  size_t listedTotal = 0;
  for (uint8_t colorIdx = 0; colorIdx < colorsCount; ++colorIdx) {
    const auto color = static_cast<Color>(colorIdx);
    for (auto kindIdx = std::to_underlying(PieceKind::Pawn);
         kindIdx < pieceKindsCount; ++kindIdx) {
      const auto piece = makePiece(color, static_cast<PieceKind>(kindIdx));
      const auto listed = board.squares(piece);
      for (const auto sqr : listed) {
        if (sqr.isInvalid() || board.pieceAt(sqr) != piece) {
          __builtin_trap();
        }
      }
      listedTotal += listed.size();
    }
  }

  size_t occupiedTotal = 0;
  for (uint8_t idx = 0; idx < 0x80; ++idx) {
    const auto sqr = Square::fromIndex(idx);
    if (sqr.isInvalid()) {
      continue;
    }
    const auto piece = board.pieceAt(sqr);
    if (piece == Piece::None) {
      continue;
    }
    ++occupiedTotal;
    const auto listed = board.squares(piece);
    if (std::ranges::find(listed, sqr) == listed.end()) {
      __builtin_trap();
    }
  }

  if (occupiedTotal != listedTotal) {
    __builtin_trap();
  }
}

/// Every FEN field must reach the board unchanged.
void checkBoardMatchesFen(const Board &board, const Fen &fen) {
  if (board.sideToMove() != fen.sideToMove() ||
      board.castlingRights() != fen.castlingAbility() ||
      board.enPassantTargetSquare() != fen.enPassantTargetSquare() ||
      board.plyClock() != fen.halfMoveClock() ||
      board.moveCounter() != fen.fullMoveCounter()) {
    __builtin_trap();
  }

  for (uint8_t idx = 0; idx < 0x80; ++idx) {
    const auto sqr = Square::fromIndex(idx);
    if (sqr.isValid() && board.pieceAt(sqr) != fen.piecePlacement().at(sqr)) {
      __builtin_trap();
    }
  }
}

} // namespace

extern "C" auto LLVMFuzzerTestOneInput(const uint8_t *data, const size_t size)
    -> int {
  const std::string_view input{reinterpret_cast<const char *>(data), size};

  const auto fen = Fen::parse(input);
  if (!fen) {
    return 0;
  }

  const auto board = Board::fromFen(*fen);
  if (!board) {
    return 0;
  }

  checkMailboxMatchesPieceLists(*board);
  checkBoardMatchesFen(*board, *fen);

  return 0;
}
