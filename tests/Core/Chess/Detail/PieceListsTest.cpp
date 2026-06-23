#include <Chelssy/Chess/Detail/PieceLists.h>
#include <gtest/gtest.h>

using namespace Chelssy::Chess;
using Detail::PieceLists;

TEST(PieceListsTest, defaultConstructedIsEmpty) {
  // arrange
  const PieceLists lists{};

  // act & assert
  for (uint8_t colorIdx = 0; colorIdx < colorsCount; ++colorIdx) {
    const auto color = static_cast<Color>(colorIdx);
    for (uint8_t kindIdx = 0; kindIdx < pieceKindsCount; ++kindIdx) {
      const auto kind = static_cast<PieceKind>(kindIdx);
      EXPECT_EQ(0, lists.count(color, kind));
      EXPECT_TRUE(lists.squares(color, kind).empty());
    }
  }
}

TEST(PieceListsTest, addAppendsSquaresInOrder) {
  // arrange
  PieceLists lists{};

  // act
  lists.add(Piece::WhiteKnight, Square{1, 0});
  lists.add(Piece::WhiteKnight, Square{6, 0});

  // assert
  EXPECT_EQ(2, lists.count(Piece::WhiteKnight));
  const auto knights = lists.squares(Color::White, PieceKind::Knight);
  ASSERT_EQ(2U, knights.size());
  EXPECT_EQ((Square{1, 0}), knights[0]);
  EXPECT_EQ((Square{6, 0}), knights[1]);
}

TEST(PieceListsTest, addSeparatesColorsAndKinds) {
  // arrange
  PieceLists lists{};

  // act
  lists.add(Piece::WhitePawn, Square{0, 1});
  lists.add(Piece::BlackPawn, Square{0, 6});
  lists.add(Piece::WhiteQueen, Square{3, 0});

  // assert
  EXPECT_EQ(1, lists.count(Piece::WhitePawn));
  EXPECT_EQ(1, lists.count(Piece::BlackPawn));
  EXPECT_EQ(1, lists.count(Piece::WhiteQueen));
  EXPECT_EQ(0, lists.count(Piece::BlackQueen));
  const auto blackPawns = lists.squares(Color::Black, PieceKind::Pawn);
  ASSERT_EQ(1U, blackPawns.size());
  EXPECT_EQ((Square{0, 6}), blackPawns.front());
}
