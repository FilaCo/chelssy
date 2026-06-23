#include "Chelssy/Chess/Fen.h"
#include <format>
#include <gtest/gtest.h>
#include <ostream>
#include <utility>

using namespace Chelssy::Chess;

static_assert(*Fen::parse(Fen::startingPositionStr) == Fen::startingPosition());

namespace {

struct FenParseTestParam {
  std::string_view name;
  std::string_view input;

  friend void PrintTo(const FenParseTestParam &param, std::ostream *out) {
    *out << std::format("{{ name: {}, input: {} }}", param.name, param.input);
  }
};

struct FenParseTest : testing::TestWithParam<FenParseTestParam> {};

} // namespace

TEST_P(FenParseTest, parseThenToChars) {
  // arrange
  const auto &[_, input] = GetParam();

  // act
  const auto parsed = Fen::parse(input);

  // assert
  ASSERT_TRUE(parsed);
  std::array<char, Fen::strLengthMax> buf{};
  const auto [ptr, ec] = parsed->toChars(buf.data(), buf.data() + buf.size());
  ASSERT_EQ(std::errc{}, ec);
  EXPECT_EQ(input, (std::string_view{buf.data(), ptr}));
}

INSTANTIATE_TEST_SUITE_P(
    Positions, FenParseTest,
    testing::Values(
        FenParseTestParam{"starting_position", Fen::startingPositionStr},
        FenParseTestParam{
            "no_white_queen",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNB1KBNR w KQq - 1 1"},
        FenParseTestParam{"kiwipite",
                          "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/"
                          "PPPBBPPP/R3K2R w KQkq - 0 1"},
        /// @see https://www.chessprogramming.org/Perft_Results
        FenParseTestParam{"cpw_pos3",
                          "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"},
        FenParseTestParam{
            "cpw_pos4",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"},
        FenParseTestParam{
            "cpw_pos4_mirrored",
            "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1"},
        FenParseTestParam{
            "cpw_pos5",
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"},
        FenParseTestParam{"cpw_pos6",
                          "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/"
                          "1PP1QPPP/R4RK1 w - - 0 10"},
        FenParseTestParam{"en_passant_white_pawn",
                          "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b "
                          "KQkq e3 0 1"},
        FenParseTestParam{"en_passant_black_pawn",
                          "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR "
                          "w KQkq e6 0 2"},
        FenParseTestParam{"partial_castling",
                          "r3k3/8/8/8/8/8/8/4K2R w Kq - 0 1"},
        FenParseTestParam{"max_counters", "8/8/8/8/8/8/8/8 w - - 100 8850"},
        // Syntactically maximal FEN (90 chars); pins str_length_max, since
        // the round trip below writes into a str_length_max buffer.
        FenParseTestParam{"worst_case_length",
                          "qqqqqqqq/qqqqqqqq/qqqqqqqq/qqqqqqqq/qqqqqqqq/"
                          "qqqqqqqq/qqqqqqqq/qqqqqqqq b KQkq a3 100 8850"}),
    [](const auto &info) -> auto { return std::string(info.param.name); });

namespace {

struct FenParseErrorTestParam {
  std::string_view name;
  std::string_view input;
  Fen::Error expected;

  friend void PrintTo(const FenParseErrorTestParam &param, std::ostream *out) {
    constexpr std::array errorStr{
        "Fen::Error::InvalidFieldsCount",
        "Fen::Error::InvalidPiecePlacementValue",
        "Fen::Error::InvalidSideToMoveValue",
        "Fen::Error::InvalidCastlingAbilityValue",
        "Fen::Error::InvalidEnPassantTargetSquareValue",
        "Fen::Error::InvalidHalfMoveClockValue",
        "Fen::Error::InvalidFullMoveCounterValue",
    };
    *out << std::format("{{ name: {}, input: {}, expected error: {} }}",
                        param.name, param.input,
                        errorStr[std::to_underlying(param.expected)]);
  }
};

struct FenParseErrorTest : testing::TestWithParam<FenParseErrorTestParam> {};

} // namespace

TEST_P(FenParseErrorTest, parseReturnsExpectedError) {
  // arrange
  const auto &[_, input, expected] = GetParam();

  // act
  const auto parsed = Fen::parse(input);

  // assert
  ASSERT_FALSE(parsed);
  EXPECT_EQ(expected, parsed.error());
}

using enum Fen::Error;

INSTANTIATE_TEST_SUITE_P(
    InvalidInputs, FenParseErrorTest,
    testing::Values(
        // field structure
        FenParseErrorTestParam{"empty", "", InvalidFieldsCount},
        FenParseErrorTestParam{"five_fields", "8/8/8/8/8/8/8/8 w - - 0",
                               InvalidFieldsCount},
        FenParseErrorTestParam{"seven_fields", "8/8/8/8/8/8/8/8 w - - 0 1 x",
                               InvalidFieldsCount},
        FenParseErrorTestParam{"double_space", "8/8/8/8/8/8/8/8  w - - 0 1",
                               InvalidFieldsCount},

        // piece placement
        FenParseErrorTestParam{"placement_seven_ranks",
                               "8/8/8/8/8/8/8 w - - 0 1",
                               InvalidPiecePlacementValue},
        FenParseErrorTestParam{"placement_nine_ranks",
                               "8/8/8/8/8/8/8/8/8 w - - 0 1",
                               InvalidPiecePlacementValue},
        FenParseErrorTestParam{"placement_rank_underfull",
                               "7/8/8/8/8/8/8/8 w - - 0 1",
                               InvalidPiecePlacementValue},
        FenParseErrorTestParam{"placement_rank_overfull",
                               "ppppppppp/8/8/8/8/8/8/8 w - - 0 1",
                               InvalidPiecePlacementValue},
        FenParseErrorTestParam{"placement_two_digits_in_row",
                               "44/8/8/8/8/8/8/8 w - - 0 1",
                               InvalidPiecePlacementValue},
        FenParseErrorTestParam{"placement_digit_nine",
                               "9/8/8/8/8/8/8/8 w - - 0 1",
                               InvalidPiecePlacementValue},
        FenParseErrorTestParam{"placement_invalid_char",
                               "x7/8/8/8/8/8/8/8 w - - 0 1",
                               InvalidPiecePlacementValue},

        // side to move
        FenParseErrorTestParam{"stm_uppercase", "8/8/8/8/8/8/8/8 W - - 0 1",
                               InvalidSideToMoveValue},
        FenParseErrorTestParam{"stm_two_chars", "8/8/8/8/8/8/8/8 wb - - 0 1",
                               InvalidSideToMoveValue},

        // castling ability
        FenParseErrorTestParam{"castling_duplicate",
                               "8/8/8/8/8/8/8/8 w KK - 0 1",
                               InvalidCastlingAbilityValue},
        FenParseErrorTestParam{"castling_invalid_char",
                               "8/8/8/8/8/8/8/8 w A - 0 1",
                               InvalidCastlingAbilityValue},
        FenParseErrorTestParam{"castling_dash_with_flag",
                               "8/8/8/8/8/8/8/8 w -K - 0 1",
                               InvalidCastlingAbilityValue},

        // en passant target square
        FenParseErrorTestParam{"ep_wrong_rank", "8/8/8/8/8/8/8/8 w - e4 0 1",
                               InvalidEnPassantTargetSquareValue},
        FenParseErrorTestParam{"ep_file_out_of_range",
                               "8/8/8/8/8/8/8/8 w - i3 0 1",
                               InvalidEnPassantTargetSquareValue},
        FenParseErrorTestParam{"ep_three_chars", "8/8/8/8/8/8/8/8 w - e33 0 1",
                               InvalidEnPassantTargetSquareValue},

        // half-move clock
        FenParseErrorTestParam{"hmc_uint8_overflow",
                               "8/8/8/8/8/8/8/8 w - - 256 1",
                               InvalidHalfMoveClockValue},
        FenParseErrorTestParam{"hmc_trailing_garbage",
                               "8/8/8/8/8/8/8/8 w - - 1x 1",
                               InvalidHalfMoveClockValue},
        FenParseErrorTestParam{"hmc_negative", "8/8/8/8/8/8/8/8 w - - -1 1",
                               InvalidHalfMoveClockValue},

        // full-move counter
        FenParseErrorTestParam{"fmc_uint16_overflow",
                               "8/8/8/8/8/8/8/8 w - - 0 65536",
                               InvalidFullMoveCounterValue}),
    [](const auto &info) -> auto { return std::string(info.param.name); });

TEST(FenToCharsTest, bufferExactlySufficient) {
  // arrange
  const auto fen = Fen::startingPosition();
  std::array<char, Fen::startingPositionStr.size()> buf{};

  // act
  const auto [ptr, ec] = fen.toChars(buf.data(), buf.data() + buf.size());

  // assert
  ASSERT_EQ(std::errc{}, ec);
  EXPECT_EQ(Fen::startingPositionStr, (std::string_view{buf.data(), ptr}));
}

TEST(FenToCharsTest, bufferOneCharShort) {
  // arrange
  const auto fen = Fen::startingPosition();
  std::array<char, Fen::startingPositionStr.size() - 1> buf{};

  // act
  const auto [ptr, ec] = fen.toChars(buf.data(), buf.data() + buf.size());

  // assert
  EXPECT_EQ(std::errc::value_too_large, ec);
}
