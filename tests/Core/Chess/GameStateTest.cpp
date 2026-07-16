#include "Chelssy/Chess/GameState.h"
#include "BoardAssertions.h"
#include "Printing.h" // IWYU pragma: keep
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <format>
#include <string>
#include <string_view>

using namespace Chelssy::Chess;

// The side to move is the one whose king matters: white stands in check
// from the adjacent rook, while the starting position is quiet.
static_assert([]() -> bool {
  const auto checked =
      *Board::fromFen(*Fen::parse("4k3/8/8/8/8/8/4r3/4K3 w - - 0 1"));
  const auto quiet = *Board::fromFen(Fen::startingPosition());
  return isInCheck(checked) && !isInCheck(quiet);
}());

// A ply that gives check is legal.
static_assert([]() -> bool {
  auto board = *Board::fromFen(*Fen::parse("4k3/8/8/8/8/8/8/4K2R w - - 0 1"));
  return isLegal(board, Ply(Square::fromStr("h1"), Square::fromStr("h8"),
                            Ply::Flag::Quiet));
}());

// The e2 knight is pinned to its king by the e3 rook and may not move;
// the king itself may step off the pin file.
static_assert([]() -> bool {
  auto board =
      *Board::fromFen(*Fen::parse("4k3/8/8/8/8/4r3/4N3/4K3 w - - 0 1"));
  return !isLegal(board, Ply(Square::fromStr("e2"), Square::fromStr("c3"),
                             Ply::Flag::Quiet)) &&
         isLegal(board, Ply(Square::fromStr("e1"), Square::fromStr("d1"),
                            Ply::Flag::Quiet));
}());

// In check from an adjacent undefended rook: capturing it or sidestepping
// off its lines is legal, stepping onto its rank is not.
static_assert([]() -> bool {
  auto board = *Board::fromFen(*Fen::parse("4k3/8/8/8/8/8/4r3/4K3 w - - 0 1"));
  return isLegal(board, Ply(Square::fromStr("e1"), Square::fromStr("e2"),
                            Ply::Flag::Capture)) &&
         !isLegal(board, Ply(Square::fromStr("e1"), Square::fromStr("d2"),
                             Ply::Flag::Quiet)) &&
         isLegal(board, Ply(Square::fromStr("e1"), Square::fromStr("d1"),
                            Ply::Flag::Quiet));
}());

namespace {

struct GameStateTestParam {
  std::string_view name;
  std::string_view fen;
  GameState expected;

  friend void PrintTo(const GameStateTestParam &param, std::ostream *out) {
    *out << std::format("{{ name: {}, fen: {} }}", param.name, param.fen);
  }
};

struct GameStateTest : testing::TestWithParam<GameStateTestParam> {};

[[nodiscard]] auto gameStateCases() {
  return testing::Values(
      GameStateTestParam{.name = "starting_position",
                         .fen = Fen::startingPositionStr,
                         .expected = GameState::Ongoing},
      // 1. f3 e5 2. g4 Qh4#
      GameStateTestParam{.name = "fools_mate",
                         .fen = "rnb1kbnr/pppp1ppp/8/4p3/6Pq/"
                                "5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3",
                         .expected = GameState::WhiteCheckmated},
      // 1. e4 e5 2. Bc4 Nc6 3. Qh5 Nf6 4. Qxf7#
      GameStateTestParam{.name = "scholars_mate",
                         .fen = "r1bqkb1r/pppp1Qpp/2n2n2/4p3/"
                                "2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4",
                         .expected = GameState::BlackCheckmated},
      // The f7 pawn and its escort take every flight square without
      // giving check.
      GameStateTestParam{.name = "pawn_stalemate",
                         .fen = "5k2/5P2/5K2/8/8/8/8/8 b - - 0 1",
                         .expected = GameState::Stalemate},
      GameStateTestParam{.name = "filled_clock_draws",
                         .fen = "k7/8/8/8/8/8/8/K6R w - - 100 60",
                         .expected = GameState::FiftyMoveDraw},
      GameStateTestParam{.name = "clock_one_short_is_ongoing",
                         .fen = "k7/8/8/8/8/8/8/K6R w - - 99 60",
                         .expected = GameState::Ongoing},
      // Terminal positions win over the filled clock: the clock is only
      // consulted when a legal ply exists.
      GameStateTestParam{.name = "mate_beats_filled_clock",
                         .fen = "7k/5KQ1/8/8/8/8/8/8 b - - 100 80",
                         .expected = GameState::BlackCheckmated},
      GameStateTestParam{.name = "stalemate_beats_filled_clock",
                         .fen = "5k2/5P2/5K2/8/8/8/8/8 b - - 100 90",
                         .expected = GameState::Stalemate});
}

struct IsLegalTestParam {
  std::string_view name;
  std::string_view fen;
  /// Perft(1) of the position: how many generated plies are legal.
  size_t legalCount;

  friend void PrintTo(const IsLegalTestParam &param, std::ostream *out) {
    *out << std::format("{{ name: {}, fen: {} }}", param.name, param.fen);
  }
};

struct IsLegalTest : testing::TestWithParam<IsLegalTestParam> {};

/// Depth-1 node counts from https://www.chessprogramming.org/Perft_Results
[[nodiscard]] auto isLegalCases() {
  return testing::Values(
      IsLegalTestParam{
          .name = "initial", .fen = Fen::startingPositionStr, .legalCount = 20},
      IsLegalTestParam{.name = "kiwipete",
                       .fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/"
                              "2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
                       .legalCount = 48},
      IsLegalTestParam{.name = "cpw_pos3",
                       .fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
                       .legalCount = 14},
      IsLegalTestParam{.name = "cpw_pos4",
                       .fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/"
                              "q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
                       .legalCount = 6},
      IsLegalTestParam{.name = "cpw_pos5",
                       .fen = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/"
                              "PPP1NnPP/RNBQK2R w KQ - 1 8",
                       .legalCount = 44},
      IsLegalTestParam{.name = "cpw_pos6",
                       .fen = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/"
                              "P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
                       .legalCount = 46});
}

} // namespace

TEST_P(GameStateTest, stateMatchesKnownPositions) {
  // arrange
  auto board = boardFromFenStr(GetParam().fen);
  ASSERT_TRUE(board.has_value());
  const auto before = *board;

  // act
  const auto state = gameState(*board);

  // assert
  EXPECT_EQ(GetParam().expected, state);
  expectBoardsEqual(before, *board);
}

INSTANTIATE_TEST_SUITE_P(Positions, GameStateTest, gameStateCases(),
                         [](const auto &info) -> auto {
                           return std::string(info.param.name);
                         });

TEST_P(IsLegalTest, legalSubsetSizeMatchesPerftDepthOne) {
  // arrange
  auto board = boardFromFenStr(GetParam().fen);
  ASSERT_TRUE(board.has_value());
  const auto before = *board;
  std::array<Ply, pliesPerPositionMax> buf;

  // act
  const auto generated = genPlies<GenKind::All>(*board, buf);
  const auto legal = std::ranges::count_if(
      generated, [&](const Ply ply) -> bool { return isLegal(*board, ply); });

  // assert
  EXPECT_EQ(GetParam().legalCount, static_cast<size_t>(legal));
  expectBoardsEqual(before, *board);
}

INSTANTIATE_TEST_SUITE_P(Positions, IsLegalTest, isLegalCases(),
                         [](const auto &info) -> auto {
                           return std::string(info.param.name);
                         });
