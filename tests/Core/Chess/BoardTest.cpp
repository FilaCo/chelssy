#include "Chelssy/Chess/Board.h"
#include "Printing.h" // IWYU pragma: keep
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <format>
#include <string_view>
#include <utility>

using namespace Chelssy::Chess;

static_assert(Board::fromFen(Fen::startingPosition()).has_value());
static_assert(Board::fromFen(*Fen::parse("8/8/8/8/8/8/8/8 w - - 0 1"))
                  .error() == Board::Error::KingMissing);

namespace {

/// @pre `str` is a syntactically valid FEN.
[[nodiscard]] auto boardFromFenStr(const std::string_view str)
    -> std::expected<Board, Board::Error> {
  const auto fen = Fen::parse(str);
  EXPECT_TRUE(fen.has_value()) << "FEN must be syntactically valid: " << str;
  return Board::fromFen(fen.value());
}

/// Checks the mailbox <-> piece-lists consistency invariant: every listed
/// square holds exactly the listed piece, every occupied mailbox square is
/// listed, and the totals match (which also rules out duplicates).
void expectMailboxMatchesPieceLists(const Board &board) {
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
    EXPECT_NE(std::ranges::find(listed, sqr), listed.end())
        << "square is occupied but not listed";
  }

  EXPECT_EQ(occupiedTotal, listedTotal);
}

} // namespace

TEST(BoardFromFenTest, startingPositionAccessors) {
  // act
  const auto board = Board::fromFen(Fen::startingPosition());

  // assert
  ASSERT_TRUE(board.has_value());
  EXPECT_EQ(Color::White, board->sideToMove());
  EXPECT_EQ(CastlingRights::All, board->castlingRights());
  EXPECT_TRUE(board->enPassantTargetSquare().isInvalid());
  EXPECT_EQ(0, board->plyClock());
  EXPECT_EQ(1, board->moveCounter());
  EXPECT_EQ(Piece::WhiteKing, board->pieceAt(Square{4, 0}));
  EXPECT_EQ(Piece::BlackQueen, board->pieceAt(Square{3, 7}));
  EXPECT_EQ(Piece::WhitePawn, board->pieceAt(Square{0, 1}));
  EXPECT_EQ(Piece::None, board->pieceAt(Square{4, 3}));
}

TEST(BoardFromFenTest, startingPositionPieceCounts) {
  // arrange
  constexpr std::array<size_t, pieceKindsCount> expectedCounts{0, 8, 2, 2,
                                                               2, 1, 1};

  // act
  const auto board = Board::fromFen(Fen::startingPosition());

  // assert
  ASSERT_TRUE(board.has_value());
  for (uint8_t colorIdx = 0; colorIdx < colorsCount; ++colorIdx) {
    const auto color = static_cast<Color>(colorIdx);
    for (auto kindIdx = std::to_underlying(PieceKind::Pawn);
         kindIdx < pieceKindsCount; ++kindIdx) {
      const auto kind = static_cast<PieceKind>(kindIdx);
      const auto piece = makePiece(color, kind);
      EXPECT_EQ(expectedCounts[kindIdx], board->squares(piece).size());
    }
  }
}

TEST(BoardFromFenTest, fenFieldsReachBoard) {
  // act
  const auto board = boardFromFenStr("r3k3/8/8/8/4P3/8/8/4K2R b Kq e3 0 42");

  // assert
  ASSERT_TRUE(board.has_value());
  EXPECT_EQ(Color::Black, board->sideToMove());
  EXPECT_EQ(CastlingRights::WhiteKing | CastlingRights::BlackQueen,
            board->castlingRights());
  EXPECT_EQ((Square{4, 2}), board->enPassantTargetSquare());
  EXPECT_EQ(0, board->plyClock());
  EXPECT_EQ(42, board->moveCounter());
}

TEST(BoardSquaresTest, startingPositionWhitePawnSquares) {
  // arrange
  constexpr std::array expected{Square{0, 1}, Square{1, 1}, Square{2, 1},
                                Square{3, 1}, Square{4, 1}, Square{5, 1},
                                Square{6, 1}, Square{7, 1}};

  // act
  const auto board = Board::fromFen(Fen::startingPosition());

  // assert
  ASSERT_TRUE(board.has_value());
  const auto pawns = board->squares(Piece::WhitePawn);
  std::array<Square, expected.size()> actual{};
  ASSERT_EQ(expected.size(), pawns.size());
  std::ranges::copy(pawns, actual.begin());
  // The listing order is an implementation detail; compare as sets.
  std::ranges::sort(actual, {}, &Square::index);
  EXPECT_TRUE(std::ranges::equal(expected, actual));
}

namespace {

struct BoardFromFenValidTestParam {
  std::string_view name;
  std::string_view input;

  friend void PrintTo(const BoardFromFenValidTestParam &param,
                      std::ostream *out) {
    *out << std::format("{{ name: {}, input: {} }}", param.name, param.input);
  }
};

struct BoardFromFenValidTest
    : testing::TestWithParam<BoardFromFenValidTestParam> {};

} // namespace

TEST_P(BoardFromFenValidTest, boardIsCreatedAndConsistent) {
  // arrange
  const auto &[_, input] = GetParam();

  // act
  const auto board = boardFromFenStr(input);

  // assert
  ASSERT_TRUE(board.has_value());
  expectMailboxMatchesPieceLists(*board);
}

INSTANTIATE_TEST_SUITE_P(
    Positions, BoardFromFenValidTest,
    testing::Values(
        BoardFromFenValidTestParam{"starting_position",
                                   Fen::startingPositionStr},
        BoardFromFenValidTestParam{"kiwipete",
                                   "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/"
                                   "2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"},
        /// @see https://www.chessprogramming.org/Perft_Results
        BoardFromFenValidTestParam{"cpw_pos3",
                                   "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"},
        BoardFromFenValidTestParam{
            "cpw_pos5",
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"},
        BoardFromFenValidTestParam{"promoted_queens",
                                   "qqq1k3/8/8/8/8/8/8/QQQ1K3 w - - 0 1"},
        // Boundary values that must pass validation.
        BoardFromFenValidTestParam{"ten_knights",
                                   "4k3/8/8/8/8/NN6/NNNNNNNN/4K3 w - - 0 1"},
        BoardFromFenValidTestParam{"nine_queens",
                                   "4k3/8/8/8/8/Q7/QQQQQQQQ/4K3 w - - 0 1"},
        BoardFromFenValidTestParam{"full_castling",
                                   "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"},
        BoardFromFenValidTestParam{"ep_white_to_move",
                                   "4k3/8/8/4p3/8/8/8/4K3 w - e6 0 2"},
        BoardFromFenValidTestParam{"ep_black_to_move",
                                   "4k3/8/8/8/4P3/8/8/4K3 b - e3 0 1"},
        BoardFromFenValidTestParam{"ply_clock_max",
                                   "4k3/8/8/8/8/8/8/4K3 w - - 100 60"},
        BoardFromFenValidTestParam{"move_counter_min",
                                   "4k3/8/8/8/8/8/8/4K3 w - - 0 1"},
        BoardFromFenValidTestParam{"move_counter_max",
                                   "4k3/8/8/8/8/8/8/4K3 w - - 0 8850"}),
    [](const auto &info) -> auto { return std::string(info.param.name); });

namespace {

struct BoardFromFenErrorTestParam {
  std::string_view name;
  std::string_view input;
  Board::Error expected;

  friend void PrintTo(const BoardFromFenErrorTestParam &param,
                      std::ostream *out) {
    constexpr std::array errorStr = {
        "Board::Error::TooManyPawns",
        "Board::Error::TooManyKnights",
        "Board::Error::TooManyBishops",
        "Board::Error::TooManyRooks",
        "Board::Error::TooManyQueens",
        "Board::Error::TooManyKings",
        "Board::Error::KingMissing",
        "Board::Error::WhitePawnOnFirstRank",
        "Board::Error::BlackPawnOnLastRank",
        "Board::Error::InvalidPlyClock",
        "Board::Error::InvalidMoveCounter",
        "Board::Error::InvalidCastlingRights",
        "Board::Error::InvalidEnPassantTargetSquare",
        "Board::Error::NonZeroPlyClockWithEnPassant",
    };

    *out << std::format("{{ name: {}, input: {}, expected error: {} }}",
                        param.name, param.input,
                        errorStr[std::to_underlying(param.expected)]);
  }
};

struct BoardFromFenErrorTest
    : testing::TestWithParam<BoardFromFenErrorTestParam> {};

} // namespace

TEST_P(BoardFromFenErrorTest, fromFenReturnsExpectedError) {
  // arrange
  const auto &[_, input, expected] = GetParam();

  // act
  const auto board = boardFromFenStr(input);

  // assert
  ASSERT_FALSE(board.has_value());
  EXPECT_EQ(expected, board.error());
}

using enum Board::Error;

INSTANTIATE_TEST_SUITE_P(
    InvalidPositions, BoardFromFenErrorTest,
    testing::Values(
        // piece count limits
        BoardFromFenErrorTestParam{"nine_pawns",
                                   "4k3/8/8/8/8/P7/PPPPPPPP/4K3 w - - 0 1",
                                   TooManyPawns},
        BoardFromFenErrorTestParam{"eleven_knights",
                                   "4k3/8/8/8/8/NNN5/NNNNNNNN/4K3 w - - 0 1",
                                   TooManyKnights},
        BoardFromFenErrorTestParam{"eleven_bishops",
                                   "4k3/8/8/8/8/BBB5/BBBBBBBB/4K3 w - - 0 1",
                                   TooManyBishops},
        BoardFromFenErrorTestParam{"eleven_rooks",
                                   "4k3/8/8/8/8/RRR5/RRRRRRRR/4K3 w - - 0 1",
                                   TooManyRooks},
        BoardFromFenErrorTestParam{"ten_queens",
                                   "4k3/8/8/8/8/QQ6/QQQQQQQQ/4K3 w - - 0 1",
                                   TooManyQueens},
        BoardFromFenErrorTestParam{
            "two_white_kings", "4k3/8/8/8/8/8/8/K3K3 w - - 0 1", TooManyKings},

        // king presence
        BoardFromFenErrorTestParam{"white_king_missing",
                                   "4k3/8/8/8/8/8/8/8 w - - 0 1", KingMissing},
        BoardFromFenErrorTestParam{"black_king_missing",
                                   "8/8/8/8/8/8/8/4K3 w - - 0 1", KingMissing},

        // pawn ranks
        BoardFromFenErrorTestParam{"white_pawn_on_first_rank",
                                   "4k3/8/8/8/8/8/8/P3K3 w - - 0 1",
                                   WhitePawnOnFirstRank},
        BoardFromFenErrorTestParam{"black_pawn_on_last_rank",
                                   "p3k3/8/8/8/8/8/8/4K3 w - - 0 1",
                                   BlackPawnOnLastRank},

        // castling rights vs placement
        BoardFromFenErrorTestParam{"castling_rook_missing",
                                   "4k3/8/8/8/8/8/8/4K3 w K - 0 1",
                                   InvalidCastlingRights},
        BoardFromFenErrorTestParam{"castling_king_displaced",
                                   "4k3/8/8/8/8/8/8/K6R w K - 0 1",
                                   InvalidCastlingRights},
        BoardFromFenErrorTestParam{"castling_queen_on_rook_square",
                                   "4k3/8/8/8/8/8/8/4K2Q w K - 0 1",
                                   InvalidCastlingRights},
        BoardFromFenErrorTestParam{"castling_black_rook_missing",
                                   "4k3/8/8/8/8/8/8/R3K2R w KQq - 0 1",
                                   InvalidCastlingRights},

        // en passant vs placement and side to move
        BoardFromFenErrorTestParam{"ep_wrong_side_to_move",
                                   "4k3/8/8/8/4P3/8/8/4K3 w - e3 0 1",
                                   InvalidEnPassantTargetSquare},
        BoardFromFenErrorTestParam{"ep_target_occupied",
                                   "4k3/8/4n3/4p3/8/8/8/4K3 w - e6 0 1",
                                   InvalidEnPassantTargetSquare},
        BoardFromFenErrorTestParam{"ep_origin_occupied",
                                   "4k3/4q3/8/4p3/8/8/8/4K3 w - e6 0 1",
                                   InvalidEnPassantTargetSquare},
        BoardFromFenErrorTestParam{"ep_pawn_missing",
                                   "4k3/8/8/8/8/8/8/4K3 w - e6 0 1",
                                   InvalidEnPassantTargetSquare},
        BoardFromFenErrorTestParam{"ep_with_nonzero_ply_clock",
                                   "4k3/8/8/4p3/8/8/8/4K3 w - e6 3 2",
                                   NonZeroPlyClockWithEnPassant},

        // counters
        BoardFromFenErrorTestParam{"ply_clock_over_max",
                                   "4k3/8/8/8/8/8/8/4K3 w - - 101 1",
                                   InvalidPlyClock},
        BoardFromFenErrorTestParam{"move_counter_zero",
                                   "4k3/8/8/8/8/8/8/4K3 w - - 0 0",
                                   InvalidMoveCounter},
        BoardFromFenErrorTestParam{"move_counter_over_max",
                                   "4k3/8/8/8/8/8/8/4K3 w - - 0 8851",
                                   InvalidMoveCounter}),
    [](const auto &info) -> auto { return std::string(info.param.name); });
