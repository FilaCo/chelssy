#include "Chelssy/Chess/Types.h"
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>

#include "Printing.h" // IWYU pragma: keep

using namespace Chelssy::Chess;

TEST(SquareTest, defaultConstructedIsInvalid) {
  const Square sqr{};
  EXPECT_TRUE(sqr.isInvalid());
  EXPECT_EQ(Square::none(), sqr);
}

TEST(SquareTest, printToUsesAlgebraicNotation) {
  EXPECT_EQ("f5", testing::PrintToString(Square::fromStr("f5")));
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
  constexpr Square e4 = Square::fromStr("e4");
  constexpr Square d4 = Square::fromStr("d4");
  constexpr std::array cases{
      ShiftedTestParam{.name = "e4_north_is_e5",
                       .from = e4,
                       .offset = Square::north,
                       .expected = Square::fromStr("e5")},
      ShiftedTestParam{.name = "e4_south_is_e3",
                       .from = e4,
                       .offset = Square::south,
                       .expected = Square::fromStr("e3")},
      ShiftedTestParam{.name = "e4_east_is_f4",
                       .from = e4,
                       .offset = Square::east,
                       .expected = Square::fromStr("f4")},
      ShiftedTestParam{.name = "e4_west_is_d4",
                       .from = e4,
                       .offset = Square::west,
                       .expected = Square::fromStr("d4")},
      ShiftedTestParam{.name = "e4_north_east_is_f5",
                       .from = e4,
                       .offset = Square::northEast,
                       .expected = Square::fromStr("f5")},
      ShiftedTestParam{.name = "e4_south_west_is_d3",
                       .from = e4,
                       .offset = Square::southWest,
                       .expected = Square::fromStr("d3")},
      ShiftedTestParam{.name = "e4_north_west_is_d5",
                       .from = e4,
                       .offset = Square::northWest,
                       .expected = Square::fromStr("d5")},
      ShiftedTestParam{.name = "e4_south_east_is_f3",
                       .from = e4,
                       .offset = Square::southEast,
                       .expected = Square::fromStr("f3")},
      // all 8 knight jumps from d4
      ShiftedTestParam{.name = "d4_knight_e6",
                       .from = d4,
                       .offset = Square::northNorthEast,
                       .expected = Square::fromStr("e6")},
      ShiftedTestParam{.name = "d4_knight_c6",
                       .from = d4,
                       .offset = Square::northNorthWest,
                       .expected = Square::fromStr("c6")},
      ShiftedTestParam{.name = "d4_knight_f5",
                       .from = d4,
                       .offset = Square::eastNorthEast,
                       .expected = Square::fromStr("f5")},
      ShiftedTestParam{.name = "d4_knight_b5",
                       .from = d4,
                       .offset = Square::westNorthWest,
                       .expected = Square::fromStr("b5")},
      ShiftedTestParam{.name = "d4_knight_c2",
                       .from = d4,
                       .offset = Square::southSouthWest,
                       .expected = Square::fromStr("c2")},
      ShiftedTestParam{.name = "d4_knight_e2",
                       .from = d4,
                       .offset = Square::southSouthEast,
                       .expected = Square::fromStr("e2")},
      ShiftedTestParam{.name = "d4_knight_b3",
                       .from = d4,
                       .offset = Square::westSouthWest,
                       .expected = Square::fromStr("b3")},
      ShiftedTestParam{.name = "d4_knight_f3",
                       .from = d4,
                       .offset = Square::eastSouthEast,
                       .expected = Square::fromStr("f3")},
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
      ShiftedOffBoardTestParam{.name = "h4_east",
                               .from = Square::fromStr("h4"),
                               .offset = Square::east},
      ShiftedOffBoardTestParam{.name = "a4_west",
                               .from = Square::fromStr("a4"),
                               .offset = Square::west},
      ShiftedOffBoardTestParam{.name = "e8_north",
                               .from = Square::fromStr("e8"),
                               .offset = Square::north},
      ShiftedOffBoardTestParam{.name = "e1_south",
                               .from = Square::fromStr("e1"),
                               .offset = Square::south},
      ShiftedOffBoardTestParam{.name = "h8_north_east",
                               .from = Square::fromStr("h8"),
                               .offset = Square::northEast},
      ShiftedOffBoardTestParam{.name = "a1_south_west",
                               .from = Square::fromStr("a1"),
                               .offset = Square::southWest},
      ShiftedOffBoardTestParam{.name = "b1_knight_down",
                               .from = Square::fromStr("b1"),
                               .offset = Square::southSouthWest},
      ShiftedOffBoardTestParam{.name = "g1_knight_down",
                               .from = Square::fromStr("g1"),
                               .offset = Square::southSouthEast},
  };

  for (const auto &[name, from, offset] : cases) {
    SCOPED_TRACE(name);
    EXPECT_TRUE(from.shifted(offset).isInvalid());
  }
}

TEST(SquareTest, shiftedInverseRestoresSquare) {
  // Every step offset used by move generation, in one direction.
  constexpr std::array<int8_t, 8> offsets{
      Square::east,          Square::north,          Square::northEast,
      Square::northWest,     Square::northNorthEast, Square::northNorthWest,
      Square::eastNorthEast, Square::westNorthWest};
  for (uint8_t file = 0; file <= fileMax; ++file) {
    for (uint8_t rank = 0; rank <= rankMax; ++rank) {
      const Square sqr{file, rank};
      for (const auto offset : offsets) {
        const auto negOffset = static_cast<int8_t>(-offset);
        EXPECT_EQ(sqr, sqr.shifted(offset).shifted(negOffset));
        EXPECT_EQ(sqr, sqr.shifted(negOffset).shifted(offset));
      }
    }
  }
}

TEST(SquareTest, invalidSquareIsNotNecessarilyNone) {
  // act: fall off the board next to h4
  const auto offBoard = Square::fromStr("h4").shifted(Square::east);

  // assert: invalid, yet distinct from the none() sentinel
  ASSERT_TRUE(offBoard.isInvalid());
  EXPECT_NE(Square::none(), offBoard);
}
