#include "Chelssy/Chess/PlyGen.h"
#include "BoardAssertions.h"
#include "Printing.h" // IWYU pragma: keep
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <format>
#include <span>
#include <string>
#include <string_view>
#include <utility>

using namespace Chelssy::Chess;

// Pawn attack asymmetry, knights jumping over pieces, and sliders blocked
// by the pawn wall.
static_assert([]() -> bool {
  const auto board = *Board::fromFen(Fen::startingPosition());
  return PlyGen::isSquareAttacked(board, Square::fromStr("e3"), Color::White) &&
         PlyGen::isSquareAttacked(board, Square::fromStr("f3"), Color::White) &&
         PlyGen::isSquareAttacked(board, Square::fromStr("e6"), Color::Black) &&
         !PlyGen::isSquareAttacked(board, Square::fromStr("e3"),
                                   Color::Black) &&
         !PlyGen::isSquareAttacked(board, Square::fromStr("d4"), Color::White);
}());

// A slider attacks the blocker's square but nothing behind it (the c3
// knight cuts the b4 bishop's ray to d2), and a ray never wraps around
// the board edge (the a1 rook must not reach h8 via 0x88 arithmetic).
static_assert([]() -> bool {
  const auto board =
      *Board::fromFen(*Fen::parse("4k3/8/8/8/1b6/2n5/8/R3K3 w Q - 0 1"));
  return PlyGen::isSquareAttacked(board, Square::fromStr("a8"), Color::White) &&
         PlyGen::isSquareAttacked(board, Square::fromStr("c3"), Color::Black) &&
         !PlyGen::isSquareAttacked(board, Square::fromStr("d2"),
                                   Color::Black) &&
         !PlyGen::isSquareAttacked(board, Square::fromStr("h8"), Color::White);
}());

namespace {

[[nodiscard]] auto attacksByStep(const Square from,
                                 const std::span<const int8_t> offsets,
                                 const Square target) -> bool {
  // An off-board `shifted` result never equals a valid target.
  return std::ranges::any_of(offsets, [from, target](const int8_t off) -> bool {
    return from.shifted(off) == target;
  });
}

[[nodiscard]] auto attacksByRay(const Board &board, const Square from,
                                const std::span<const int8_t> dirs,
                                const Square target) -> bool {
  for (const auto dir : dirs) {
    for (auto sqr = from.shifted(dir); sqr.isValid(); sqr = sqr.shifted(dir)) {
      if (sqr == target) {
        return true;
      }
      if (board.pieceAt(sqr) != Piece::None) {
        break;
      }
    }
  }
  return false;
}

[[nodiscard]] auto isSquareAttackedReference(const Board &board,
                                             const Square target,
                                             const Color by) -> bool {
  for (uint8_t idx = 0; idx < mailboxSize; ++idx) {
    const auto from = Square::fromIndex(idx);
    if (from.isInvalid()) {
      continue;
    }
    const auto piece = board.pieceAt(from);
    if (piece == Piece::None || getColor(piece) != by) {
      continue;
    }

    const auto attacks = [&]() -> bool {
      switch (getKind(piece)) {
      case PieceKind::Pawn:
        return attacksByStep(
            from, Square::pawnAttackOffsets[std::to_underlying(by)], target);
      case PieceKind::Knight:
        return attacksByStep(from, Square::knightOffsets, target);
      case PieceKind::King:
        return attacksByStep(from, Square::kingOffsets, target);
      case PieceKind::Bishop:
        return attacksByRay(board, from, Square::bishopOffsets, target);
      case PieceKind::Rook:
        return attacksByRay(board, from, Square::rookOffsets, target);
      case PieceKind::Queen:
        return attacksByRay(board, from, Square::bishopOffsets, target) ||
               attacksByRay(board, from, Square::rookOffsets, target);
      default:
        return false;
      }
    }();
    if (attacks) {
      return true;
    }
  }
  return false;
}

struct AttackedTestParam {
  std::string_view name;
  std::string_view fen;

  friend void PrintTo(const AttackedTestParam &param, std::ostream *out) {
    *out << std::format("{{ name: {}, fen: {} }}", param.name, param.fen);
  }
};

struct PlyGenAttackedTest : testing::TestWithParam<AttackedTestParam> {};

[[nodiscard]] auto attackedCases() {
  return testing::Values(
      AttackedTestParam{.name = "starting_position",
                        .fen = Fen::startingPositionStr},
      /// @see https://www.chessprogramming.org/Perft_Results
      AttackedTestParam{.name = "kiwipete",
                        .fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/"
                               "2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"},
      AttackedTestParam{.name = "cpw_pos3",
                        .fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"},
      AttackedTestParam{.name = "cpw_pos4",
                        .fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/"
                               "q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"},
      AttackedTestParam{
          .name = "cpw_pos5",
          .fen = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"},
      AttackedTestParam{.name = "cpw_pos6",
                        .fen = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/"
                               "P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"},
      AttackedTestParam{.name = "en_passant_pair",
                        .fen = "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 3"},
      AttackedTestParam{.name = "corner_queens",
                        .fen = "k6q/8/8/8/8/8/8/Q6K w - - 0 1"},
      AttackedTestParam{.name = "rim_knights",
                        .fen = "N6N/8/8/k6K/8/8/8/N6N w - - 0 1"},
      AttackedTestParam{.name = "lone_kings",
                        .fen = "8/8/4k3/8/8/3K4/8/8 w - - 0 1"});
}

} // namespace

TEST_P(PlyGenAttackedTest, isSquareAttackedMatchesNaiveReference) {
  // arrange
  const auto board = boardFromFenStr(GetParam().fen);
  ASSERT_TRUE(board.has_value());

  // act + assert: every square times both attacking colors
  for (uint8_t idx = 0; idx < chessboardSize; ++idx) {
    const auto sqr = Square::from8x8(idx);
    for (uint8_t colorIdx = 0; colorIdx < colorsCount; ++colorIdx) {
      const auto by = static_cast<Color>(colorIdx);
      EXPECT_EQ(isSquareAttackedReference(*board, sqr, by),
                PlyGen::isSquareAttacked(*board, sqr, by))
          << "at " << testing::PrintToString(sqr) << " by "
          << (by == Color::White ? "white" : "black");
    }
  }
}

INSTANTIATE_TEST_SUITE_P(Positions, PlyGenAttackedTest, attackedCases(),
                         [](const auto &info) -> auto {
                           return std::string(info.param.name);
                         });
