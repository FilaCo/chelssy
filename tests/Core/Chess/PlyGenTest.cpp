#include "Chelssy/Chess/PlyGen.h"
#include "BoardAssertions.h"
#include "Printing.h" // IWYU pragma: keep
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace Chelssy::Chess;

// Pawn attack asymmetry, knights jumping over pieces, and sliders blocked
// by the pawn wall.
static_assert([]() -> bool {
  const auto board = *Board::fromFen(Fen::startingPosition());
  return isSquareAttacked(board, Square::fromStr("e3"), Color::White) &&
         isSquareAttacked(board, Square::fromStr("f3"), Color::White) &&
         isSquareAttacked(board, Square::fromStr("e6"), Color::Black) &&
         !isSquareAttacked(board, Square::fromStr("e3"), Color::Black) &&
         !isSquareAttacked(board, Square::fromStr("d4"), Color::White);
}());

// A slider attacks the blocker's square but nothing behind it (the c3
// knight cuts the b4 bishop's ray to d2), and a ray never wraps around
// the board edge (the a1 rook must not reach h8 via 0x88 arithmetic).
static_assert([]() -> bool {
  const auto board =
      *Board::fromFen(*Fen::parse("4k3/8/8/8/1b6/2n5/8/R3K3 w Q - 0 1"));
  return isSquareAttacked(board, Square::fromStr("a8"), Color::White) &&
         isSquareAttacked(board, Square::fromStr("c3"), Color::Black) &&
         !isSquareAttacked(board, Square::fromStr("d2"), Color::Black) &&
         !isSquareAttacked(board, Square::fromStr("h8"), Color::White);
}());

// Starting position: sixteen pawn plies plus four knight plies, and
// none of them changes material.
static_assert([]() -> bool {
  const auto board = *Board::fromFen(Fen::startingPosition());
  std::array<Ply, pliesPerPositionMax> buf;
  return genPlies<GenKind::All>(board, buf).size() == 20 &&
         genPlies<GenKind::ChangesMaterial>(board, buf).empty();
}());

// The only material-changing ply is the en-passant capture e5xd6.
static_assert([]() -> bool {
  const auto board =
      *Board::fromFen(*Fen::parse("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 3"));
  std::array<Ply, pliesPerPositionMax> buf;
  const auto plies = genPlies<GenKind::ChangesMaterial>(board, buf);
  return plies.size() == 1 &&
         plies[0] == Ply(Square::fromStr("e5"), Square::fromStr("d6"),
                         Ply::Flag::EnPassantCapture);
}());

// A promotion push fans out into all four promotion kinds and belongs
// to the Captures subset even though nothing is taken.
static_assert([]() -> bool {
  const auto board =
      *Board::fromFen(*Fen::parse("8/4P1k1/8/8/8/8/8/4K3 w - - 0 1"));
  std::array<Ply, pliesPerPositionMax> buf;
  const auto plies = genPlies<GenKind::ChangesMaterial>(board, buf);
  return plies.size() == 4 &&
         std::ranges::all_of(plies, [](const Ply ply) -> bool {
           return ply.isPromotion() && !ply.isCapture();
         });
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
      if (!board.isEmpty(sqr)) {
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
    if (!hasColor(piece, by)) {
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

/// @see https://www.chessprogramming.org/Perft
[[nodiscard]] auto perft(Board &board, const uint32_t depth) -> uint64_t {
  if (depth == 0) {
    return 1;
  }
  std::array<Ply, pliesPerPositionMax> buf;
  const auto plies = genPlies<GenKind::All>(board, buf);
  const auto mover = board.sideToMove();
  uint64_t nodes = 0;
  for (const auto ply : plies) {
    Undo undo{};
    board.doPly(ply, undo);
    if (!isSquareAttacked(board, board.kingSquare(mover), flip(mover))) {
      nodes += perft(board, depth - 1);
    }
    board.undoPly(ply, undo);
  }
  return nodes;
}

struct PerftTestParam {
  std::string_view name;
  std::string_view fen;
  /// expected[d - 1] is the node count at depth d.
  std::span<const uint64_t> expected;

  friend void PrintTo(const PerftTestParam &param, std::ostream *out) {
    *out << std::format("{{ name: {}, fen: {} }}", param.name, param.fen);
  }
};

struct PlyGenPerftTest : testing::TestWithParam<PerftTestParam> {};

/// Node counts from https://www.chessprogramming.org/Perft_Results;
/// depths are capped to keep the sanitized Debug run fast.
constexpr std::array<uint64_t, 4> initialNodes{20, 400, 8902, 197281};
constexpr std::array<uint64_t, 3> kiwipeteNodes{48, 2039, 97862};
constexpr std::array<uint64_t, 4> cpwPos3Nodes{14, 191, 2812, 43238};
constexpr std::array<uint64_t, 3> cpwPos4Nodes{6, 264, 9467};
constexpr std::array<uint64_t, 3> cpwPos5Nodes{44, 1486, 62379};
constexpr std::array<uint64_t, 3> cpwPos6Nodes{46, 2079, 89890};

[[nodiscard]] auto perftCases() {
  return testing::Values(
      PerftTestParam{.name = "initial",
                     .fen = Fen::startingPositionStr,
                     .expected = initialNodes},
      PerftTestParam{.name = "kiwipete",
                     .fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/"
                            "2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
                     .expected = kiwipeteNodes},
      PerftTestParam{.name = "cpw_pos3",
                     .fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
                     .expected = cpwPos3Nodes},
      PerftTestParam{.name = "cpw_pos4",
                     .fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/"
                            "q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
                     .expected = cpwPos4Nodes},
      PerftTestParam{.name = "cpw_pos5",
                     .fen = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/"
                            "PPP1NnPP/RNBQK2R w KQ - 1 8",
                     .expected = cpwPos5Nodes},
      PerftTestParam{.name = "cpw_pos6",
                     .fen = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/"
                            "P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
                     .expected = cpwPos6Nodes});
}

struct PlyGenCapturesTest : testing::TestWithParam<AttackedTestParam> {};

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
                isSquareAttacked(*board, sqr, by))
          << "at " << testing::PrintToString(sqr) << " by "
          << (by == Color::White ? "white" : "black");
    }
  }
}

INSTANTIATE_TEST_SUITE_P(Positions, PlyGenAttackedTest, attackedCases(),
                         [](const auto &info) -> auto {
                           return std::string(info.param.name);
                         });

TEST_P(PlyGenPerftTest, nodeCountsMatchKnownResults) {
  // arrange
  auto board = boardFromFenStr(GetParam().fen);
  ASSERT_TRUE(board.has_value());

  // act + assert
  const auto expected = GetParam().expected;
  for (uint32_t depth = 1; depth <= expected.size(); ++depth) {
    EXPECT_EQ(expected[depth - 1], perft(*board, depth))
        << "at depth " << depth;
  }
}

INSTANTIATE_TEST_SUITE_P(Positions, PlyGenPerftTest, perftCases(),
                         [](const auto &info) -> auto {
                           return std::string(info.param.name);
                         });

TEST_P(PlyGenCapturesTest, capturesAreTheMaterialSubsetOfAll) {
  // arrange
  const auto board = boardFromFenStr(GetParam().fen);
  ASSERT_TRUE(board.has_value());
  std::array<Ply, pliesPerPositionMax> allBuf;
  std::array<Ply, pliesPerPositionMax> capturesBuf;

  // act
  const auto allPlies = genPlies<GenKind::All>(*board, allBuf);
  const auto captures = genPlies<GenKind::ChangesMaterial>(*board, capturesBuf);

  // assert: the same plies in the same relative order
  std::vector<Ply> expected;
  std::ranges::copy_if(allPlies, std::back_inserter(expected),
                       [](const Ply ply) -> bool {
                         return ply.isCapture() || ply.isPromotion();
                       });
  EXPECT_TRUE(std::ranges::equal(expected, captures));
}

INSTANTIATE_TEST_SUITE_P(Positions, PlyGenCapturesTest, attackedCases(),
                         [](const auto &info) -> auto {
                           return std::string(info.param.name);
                         });
