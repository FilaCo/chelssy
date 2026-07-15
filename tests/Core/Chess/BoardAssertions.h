#pragma once

#include "Chelssy/Chess/Board.h"
#include "Printing.h" // IWYU pragma: keep
#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>
#include <utility>

namespace Chelssy::Chess {

/// @pre `str` is a syntactically valid FEN.
[[nodiscard]] inline auto boardFromFenStr(const std::string_view str)
    -> std::expected<Board, Board::Error> {
  const auto fen = Fen::parse(str);
  EXPECT_TRUE(fen.has_value()) << "FEN must be syntactically valid: " << str;
  return Board::fromFen(fen.value());
}

inline void expectMailboxMatchesPieceLists(const Board &board) {
  size_t listedTotal = 0;
  for (uint8_t colorIdx = 0; colorIdx < colorsCount; ++colorIdx) {
    const auto color = static_cast<Color>(colorIdx);
    for (auto kindIdx = std::to_underlying(PieceKind::Pawn);
         kindIdx < pieceKindsCount; ++kindIdx) {
      const auto kind = static_cast<PieceKind>(kindIdx);
      const auto piece = makePiece(color, kind);
      const auto listed = board.squares(piece);
      for (const auto sqr : listed) {
        ASSERT_TRUE(sqr.isValid());
        EXPECT_EQ(makePiece(color, kind), board.pieceAt(sqr));
      }
      listedTotal += listed.size();
    }
  }

  size_t occupiedTotal = 0;
  for (uint8_t idx = 0; idx < mailboxSize; ++idx) {
    const auto sqr = Square::fromIndex(idx);
    if (sqr.isInvalid()) {
      continue;
    }
    const auto piece = board.pieceAt(sqr);
    if (isNone(piece)) {
      continue;
    }
    ++occupiedTotal;
    const auto listed = board.squares(piece);
    EXPECT_NE(std::ranges::find(listed, sqr), listed.end())
        << "square is occupied but not listed";
  }

  EXPECT_EQ(occupiedTotal, listedTotal);
}

/// Compares the observable state: scalars plus every valid square's piece.
/// Piece-list order is ignored.
inline void expectBoardsEqual(const Board &expected, const Board &actual) {
  EXPECT_EQ(expected.sideToMove(), actual.sideToMove());
  EXPECT_EQ(expected.castlingRights(), actual.castlingRights());
  EXPECT_EQ(expected.enPassantTargetSquare(), actual.enPassantTargetSquare());
  EXPECT_EQ(expected.plyClock(), actual.plyClock());
  EXPECT_EQ(expected.moveCounter(), actual.moveCounter());
  for (uint8_t idx = 0; idx < mailboxSize; ++idx) {
    const auto sqr = Square::fromIndex(idx);
    if (sqr.isInvalid()) {
      continue;
    }
    EXPECT_EQ(expected.pieceAt(sqr), actual.pieceAt(sqr))
        << "at " << testing::PrintToString(sqr);
  }
}

} // namespace Chelssy::Chess
