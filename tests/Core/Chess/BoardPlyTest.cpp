#include "BoardAssertions.h"
#include "Chelssy/Chess/Board.h"
#include "Printing.h" // IWYU pragma: keep
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <format>
#include <string_view>

using namespace Chelssy::Chess;

// e2-e4 applied and reverted at compile time.
static_assert([]() -> bool {
  const Ply ply{Square::fromStr("e2"), Square::fromStr("e4"),
                Ply::Flag::DoublePawnPush};
  auto board = *Board::fromFen(Fen::startingPosition());
  Undo undo{};

  board.doPly(ply, undo);
  const auto applied =
      board.pieceAt(Square::fromStr("e4")) == Piece::WhitePawn &&
      board.pieceAt(Square::fromStr("e2")) == Piece::None &&
      board.sideToMove() == Color::Black &&
      board.enPassantTargetSquare() == Square::fromStr("e3");

  board.undoPly(ply, undo);
  return applied && board.pieceAt(Square::fromStr("e2")) == Piece::WhitePawn &&
         board.pieceAt(Square::fromStr("e4")) == Piece::None &&
         board.sideToMove() == Color::White &&
         board.enPassantTargetSquare().isInvalid();
}());

namespace {

struct PlyTestParam {
  std::string_view name;
  std::string_view fenBefore;
  Ply ply;
  std::string_view fenAfter;

  friend void PrintTo(const PlyTestParam &param, std::ostream *out) {
    *out << std::format("{{ name: {}, before: {}, after: {} }}", param.name,
                        param.fenBefore, param.fenAfter);
  }
};

struct BoardPlyTest : testing::TestWithParam<PlyTestParam> {};

[[nodiscard]] auto plyCases() {
  using enum Ply::Flag;
  return testing::Values(
      PlyTestParam{
          .name = "quiet_knight_move",
          .fenBefore = Fen::startingPositionStr,
          .ply = Ply{Square::fromStr("g1"), Square::fromStr("f3"), Quiet},
          .fenAfter = "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq "
                      "- 1 1"},
      PlyTestParam{.name = "double_push_sets_ep",
                   .fenBefore = Fen::startingPositionStr,
                   .ply = Ply{Square::fromStr("e2"), Square::fromStr("e4"),
                              DoublePawnPush},
                   .fenAfter =
                       "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq "
                       "e3 0 1"},
      PlyTestParam{
          .name = "black_ply_clears_ep_and_increments_move_counter",
          .fenBefore =
              "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
          .ply = Ply{Square::fromStr("g8"), Square::fromStr("f6"), Quiet},
          .fenAfter =
              "rnbqkb1r/pppppppp/5n2/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 1 2"},
      PlyTestParam{
          .name = "capture_resets_ply_clock",
          .fenBefore = "4k3/8/8/3p4/4N3/8/8/4K3 w - - 5 20",
          .ply = Ply{Square::fromStr("e4"), Square::fromStr("d5"), Capture},
          .fenAfter = "4k3/8/8/3N4/8/8/8/4K3 b - - 0 20"},
      PlyTestParam{.name = "ep_capture_white",
                   .fenBefore = "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 3",
                   .ply = Ply{Square::fromStr("e5"), Square::fromStr("d6"),
                              EnPassantCapture},
                   .fenAfter = "4k3/8/3P4/8/8/8/8/4K3 b - - 0 3"},
      PlyTestParam{.name = "ep_capture_black",
                   .fenBefore = "4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 5",
                   .ply = Ply{Square::fromStr("d4"), Square::fromStr("e3"),
                              EnPassantCapture},
                   .fenAfter = "4k3/8/8/8/8/4p3/8/4K3 w - - 0 6"},
      PlyTestParam{
          .name = "white_king_castle",
          .fenBefore = "4k3/8/8/8/8/8/8/4K2R w K - 3 10",
          .ply = Ply{Square::fromStr("e1"), Square::fromStr("g1"), KingCastle},
          .fenAfter = "4k3/8/8/8/8/8/8/5RK1 b - - 4 10"},
      PlyTestParam{
          .name = "white_queen_castle",
          .fenBefore = "4k3/8/8/8/8/8/8/R3K3 w Q - 0 10",
          .ply = Ply{Square::fromStr("e1"), Square::fromStr("c1"), QueenCastle},
          .fenAfter = "4k3/8/8/8/8/8/8/2KR4 b - - 1 10"},
      PlyTestParam{
          .name = "black_king_castle",
          .fenBefore = "4k2r/8/8/8/8/8/8/4K3 b k - 0 10",
          .ply = Ply{Square::fromStr("e8"), Square::fromStr("g8"), KingCastle},
          .fenAfter = "5rk1/8/8/8/8/8/8/4K3 w - - 1 11"},
      PlyTestParam{
          .name = "black_queen_castle",
          .fenBefore = "r3k3/8/8/8/8/8/8/4K3 b q - 0 10",
          .ply = Ply{Square::fromStr("e8"), Square::fromStr("c8"), QueenCastle},
          .fenAfter = "2kr4/8/8/8/8/8/8/4K3 w - - 1 11"},
      PlyTestParam{
          .name = "knight_promo",
          .fenBefore = "4k3/P7/8/8/8/8/8/4K3 w - - 0 30",
          .ply = Ply{Square::fromStr("a7"), Square::fromStr("a8"), KnightPromo},
          .fenAfter = "N3k3/8/8/8/8/8/8/4K3 b - - 0 30"},
      PlyTestParam{
          .name = "bishop_promo",
          .fenBefore = "4k3/P7/8/8/8/8/8/4K3 w - - 0 30",
          .ply = Ply{Square::fromStr("a7"), Square::fromStr("a8"), BishopPromo},
          .fenAfter = "B3k3/8/8/8/8/8/8/4K3 b - - 0 30"},
      PlyTestParam{
          .name = "rook_promo",
          .fenBefore = "4k3/P7/8/8/8/8/8/4K3 w - - 0 30",
          .ply = Ply{Square::fromStr("a7"), Square::fromStr("a8"), RookPromo},
          .fenAfter = "R3k3/8/8/8/8/8/8/4K3 b - - 0 30"},
      PlyTestParam{
          .name = "queen_promo",
          .fenBefore = "4k3/P7/8/8/8/8/8/4K3 w - - 0 30",
          .ply = Ply{Square::fromStr("a7"), Square::fromStr("a8"), QueenPromo},
          .fenAfter = "Q3k3/8/8/8/8/8/8/4K3 b - - 0 30"},
      PlyTestParam{.name = "queen_promo_capture",
                   .fenBefore = "1r2k3/P7/8/8/8/8/8/4K3 w - - 0 30",
                   .ply = Ply{Square::fromStr("a7"), Square::fromStr("b8"),
                              QueenPromoCapture},
                   .fenAfter = "1Q2k3/8/8/8/8/8/8/4K3 b - - 0 30"},
      PlyTestParam{.name = "black_knight_promo_capture",
                   .fenBefore = "4k3/8/8/8/8/8/p7/1N2K3 b - - 0 30",
                   .ply = Ply{Square::fromStr("a2"), Square::fromStr("b1"),
                              KnightPromoCapture},
                   .fenAfter = "4k3/8/8/8/8/8/8/1n2K3 w - - 0 31"},
      PlyTestParam{.name = "king_move_clears_both_rights",
                   .fenBefore = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 10",
                   .ply =
                       Ply{Square::fromStr("e1"), Square::fromStr("e2"), Quiet},
                   .fenAfter = "r3k2r/8/8/8/8/8/4K3/R6R b kq - 1 10"},
      PlyTestParam{.name = "rook_move_clears_kingside_right",
                   .fenBefore = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 10",
                   .ply =
                       Ply{Square::fromStr("h1"), Square::fromStr("g1"), Quiet},
                   .fenAfter = "r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 1 10"},
      PlyTestParam{
          .name = "rook_capture_clears_both_sides_rights",
          .fenBefore = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 10",
          .ply = Ply{Square::fromStr("a1"), Square::fromStr("a8"), Capture},
          .fenAfter = "R3k2r/8/8/8/8/8/8/4K2R b Kk - 0 10"});
}

} // namespace

TEST_P(BoardPlyTest, doPlyResultMatchesExpectedPosition) {
  // arrange
  const auto &[_, fenBefore, ply, fenAfter] = GetParam();
  auto board = boardFromFenStr(fenBefore);
  const auto expected = boardFromFenStr(fenAfter);
  ASSERT_TRUE(board.has_value());
  ASSERT_TRUE(expected.has_value());
  Undo undo{};

  // act
  board->doPly(ply, undo);

  // assert
  expectBoardsEqual(*expected, *board);
  expectMailboxMatchesPieceLists(*board);
}

TEST_P(BoardPlyTest, undoPlyRestoresEquivalentPosition) {
  // arrange
  const auto &[_, fenBefore, ply, fenAfter] = GetParam();
  auto board = boardFromFenStr(fenBefore);
  const auto original = boardFromFenStr(fenBefore);
  ASSERT_TRUE(board.has_value());
  Undo undo{};

  // act
  board->doPly(ply, undo);
  board->undoPly(ply, undo);

  // assert
  expectBoardsEqual(*original, *board);
  expectMailboxMatchesPieceLists(*board);
}

INSTANTIATE_TEST_SUITE_P(Plies, BoardPlyTest, plyCases(),
                         [](const auto &info) -> auto {
                           return std::string(info.param.name);
                         });

TEST(BoardPlySequenceTest, doUndoStackRestoresOriginal) {
  // arrange
  auto board = boardFromFenStr(Fen::startingPositionStr);
  const auto original = boardFromFenStr(Fen::startingPositionStr);
  ASSERT_TRUE(board.has_value());
  using enum Ply::Flag;
  constexpr std::array plies{
      Ply{Square::fromStr("e2"), Square::fromStr("e4"), DoublePawnPush},
      Ply{Square::fromStr("d7"), Square::fromStr("d5"), DoublePawnPush},
      Ply{Square::fromStr("e4"), Square::fromStr("d5"), Capture},
      Ply{Square::fromStr("d8"), Square::fromStr("d5"), Capture},
  };
  std::array<Undo, plies.size()> undos{};

  // act
  for (size_t i = 0; i < plies.size(); ++i) {
    board->doPly(plies[i], undos[i]);
    expectMailboxMatchesPieceLists(*board);
  }
  for (size_t i = plies.size(); i-- > 0;) {
    board->undoPly(plies[i], undos[i]);
    expectMailboxMatchesPieceLists(*board);
  }

  // assert
  expectBoardsEqual(*original, *board);
}
