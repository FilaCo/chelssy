#include "Chelssy/Chess/Types.h"
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>

#include "Printing.h" // IWYU pragma: keep

using namespace Chelssy::Chess;

TEST(SquareTest, printToUsesAlgebraicNotation) {
  EXPECT_EQ("f5", testing::PrintToString(Square{5, 4}));
}

static_assert(Square(4, 3) == Square::fromIndex(0x34));

TEST(SquareTest, fileRankRoundTrip) {
  for (uint8_t file = 0; file <= fileMax; ++file) {
    for (uint8_t rank = 0; rank <= rankMax; ++rank) {
      // act
      const Square sqr{file, rank};

      // assert
      EXPECT_TRUE(sqr.isValid());
      EXPECT_EQ(file, sqr.file());
      EXPECT_EQ(rank, sqr.rank());
    }
  }
}

TEST(SquareTest, validityEnumeration) {
  // arrange
  std::array<bool, 256> expected{};
  for (uint8_t file = 0; file <= fileMax; ++file) {
    for (uint8_t rank = 0; rank <= rankMax; ++rank) {
      expected[Square{file, rank}.index()] = true;
    }
  }

  size_t validCount = 0;
  for (size_t idx = 0; idx < expected.size(); ++idx) {
    // act
    const auto sqr = Square::fromIndex(static_cast<uint8_t>(idx));

    // assert
    EXPECT_EQ(expected[idx], sqr.isValid());
    EXPECT_EQ(!expected[idx], sqr.isInvalid());
    validCount += sqr.isValid() ? 1 : 0;
  }
  EXPECT_EQ(chessboardSize, validCount);
}

TEST(SquareTest, indexRoundTrip) {
  for (size_t idx = 0; idx < 256; ++idx) {
    // act
    const auto sqr = Square::fromIndex(static_cast<uint8_t>(idx));

    // assert
    EXPECT_EQ(idx, sqr.index());
    EXPECT_EQ(sqr, Square::fromIndex(sqr.index()));
  }
}

TEST(SquareTest, to8x8MappingIsABijection) {
  std::array<bool, chessboardSize> seen{};
  for (uint8_t file = 0; file <= fileMax; ++file) {
    for (uint8_t rank = 0; rank <= rankMax; ++rank) {
      // act
      const auto sqr8x8 = Square{file, rank}.to8x8();

      // assert
      EXPECT_EQ((rank * 8) + file, sqr8x8);
      ASSERT_LT(sqr8x8, chessboardSize);
      EXPECT_FALSE(seen[sqr8x8]);
      seen[sqr8x8] = true;
    }
  }
}

namespace {

struct ShiftedTestParam {
  std::string_view name;
  Square from;
  int8_t offset;
  Square expected;
};

struct ShiftedOffBoardTestParam {
  std::string_view name;
  Square from;
  int8_t offset;
};

} // namespace

TEST(SquareTest, shiftedGeometryOnBoard) {
  // arrange
  constexpr Square e4{4, 3};
  constexpr Square d4{3, 3};
  constexpr std::array cases{
      ShiftedTestParam{.name = "e4_north_is_e5",
                       .from = e4,
                       .offset = 0x10,
                       .expected = Square{4, 4}},
      ShiftedTestParam{.name = "e4_south_is_e3",
                       .from = e4,
                       .offset = -0x10,
                       .expected = Square{4, 2}},
      ShiftedTestParam{.name = "e4_east_is_f4",
                       .from = e4,
                       .offset = 0x01,
                       .expected = Square{5, 3}},
      ShiftedTestParam{.name = "e4_west_is_d4",
                       .from = e4,
                       .offset = -0x01,
                       .expected = Square{3, 3}},
      ShiftedTestParam{.name = "e4_north_east_is_f5",
                       .from = e4,
                       .offset = 0x11,
                       .expected = Square{5, 4}},
      ShiftedTestParam{.name = "e4_south_west_is_d3",
                       .from = e4,
                       .offset = -0x11,
                       .expected = Square{3, 2}},
      ShiftedTestParam{.name = "e4_north_west_is_d5",
                       .from = e4,
                       .offset = 0x0F,
                       .expected = Square{3, 4}},
      ShiftedTestParam{.name = "e4_south_east_is_f3",
                       .from = e4,
                       .offset = -0x0F,
                       .expected = Square{5, 2}},
      // all 8 knight jumps from d4
      ShiftedTestParam{.name = "d4_knight_e6",
                       .from = d4,
                       .offset = 0x21,
                       .expected = Square{4, 5}},
      ShiftedTestParam{.name = "d4_knight_c6",
                       .from = d4,
                       .offset = 0x1F,
                       .expected = Square{2, 5}},
      ShiftedTestParam{.name = "d4_knight_f5",
                       .from = d4,
                       .offset = 0x12,
                       .expected = Square{5, 4}},
      ShiftedTestParam{.name = "d4_knight_b5",
                       .from = d4,
                       .offset = 0x0E,
                       .expected = Square{1, 4}},
      ShiftedTestParam{.name = "d4_knight_c2",
                       .from = d4,
                       .offset = -0x21,
                       .expected = Square{2, 1}},
      ShiftedTestParam{.name = "d4_knight_e2",
                       .from = d4,
                       .offset = -0x1F,
                       .expected = Square{4, 1}},
      ShiftedTestParam{.name = "d4_knight_b3",
                       .from = d4,
                       .offset = -0x12,
                       .expected = Square{1, 2}},
      ShiftedTestParam{.name = "d4_knight_f3",
                       .from = d4,
                       .offset = -0x0E,
                       .expected = Square{5, 2}},
  };

  for (const auto &[name, from, offset, expected] : cases) {
    SCOPED_TRACE(name);

    // act
    const auto actual = from.shifted(offset);

    // assert
    ASSERT_TRUE(actual.isValid());
    EXPECT_EQ(expected, actual);
  }
}

TEST(SquareTest, shiftedFallsOffBoard) {
  // arrange
  constexpr std::array cases{
      ShiftedOffBoardTestParam{
          .name = "h4_east", .from = Square{7, 3}, .offset = 0x01},
      ShiftedOffBoardTestParam{
          .name = "a4_west", .from = Square{0, 3}, .offset = -0x01},
      ShiftedOffBoardTestParam{
          .name = "e8_north", .from = Square{4, 7}, .offset = 0x10},
      ShiftedOffBoardTestParam{
          .name = "e1_south", .from = Square{4, 0}, .offset = -0x10},
      ShiftedOffBoardTestParam{
          .name = "h8_north_east", .from = Square{7, 7}, .offset = 0x11},
      ShiftedOffBoardTestParam{
          .name = "a1_south_west", .from = Square{0, 0}, .offset = -0x11},
      ShiftedOffBoardTestParam{
          .name = "b1_knight_down", .from = Square{1, 0}, .offset = -0x21},
      ShiftedOffBoardTestParam{
          .name = "g1_knight_down", .from = Square{6, 0}, .offset = -0x1F},
  };

  for (const auto &[name, from, offset] : cases) {
    SCOPED_TRACE(name);
    EXPECT_TRUE(from.shifted(offset).isInvalid());
  }
}

TEST(SquareTest, shiftedInverseRestoresSquare) {
  // Every step offset used by move generation, in one direction.
  constexpr std::array<int8_t, 8> offsets{0x01, 0x10, 0x11, 0x0F,
                                          0x21, 0x1F, 0x12, 0x0E};
  for (uint8_t file = 0; file <= fileMax; ++file) {
    for (uint8_t rank = 0; rank <= rankMax; ++rank) {
      const Square sqr{file, rank};
      for (const auto offset : offsets) {
        EXPECT_EQ(sqr, sqr.shifted(offset).shifted(-offset));
        EXPECT_EQ(sqr, sqr.shifted(-offset).shifted(offset));
      }
    }
  }
}

TEST(SquareTest, invalidSquareIsNotNecessarilyNone) {
  // act: fall off the board next to h4
  const auto offBoard = Square{7, 3}.shifted(0x01);

  // assert: invalid, yet distinct from the none() sentinel
  ASSERT_TRUE(offBoard.isInvalid());
  EXPECT_NE(Square::none(), offBoard);
}
