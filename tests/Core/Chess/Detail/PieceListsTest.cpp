#include <Chelssy/Chess/Detail/PieceLists.h>
#include <gtest/gtest.h>

#include <utility>

using namespace Chelssy::Chess;
using Detail::PieceLists;

TEST(PieceListsTest, defaultConstructedIsEmpty) {
  // arrange
  const PieceLists lists{};

  // act & assert
  for (uint8_t colorIdx = 0; colorIdx < colorsCount; ++colorIdx) {
    const auto color = static_cast<Color>(colorIdx);
    for (auto kindIdx = std::to_underlying(PieceKind::Pawn);
         kindIdx < pieceKindsCount; ++kindIdx) {
      const auto kind = static_cast<PieceKind>(kindIdx);
      const auto piece = makePiece(color, kind);
      EXPECT_EQ(0, lists.count(piece));
      EXPECT_TRUE(lists.squares(piece).empty());
    }
  }
}

TEST(PieceListsTest, addAppendsSquaresInOrder) {
  // arrange
  PieceLists lists{};

  // act
  lists.add(Piece::WhiteKnight, Square::fromStr("b1"));
  lists.add(Piece::WhiteKnight, Square::fromStr("g1"));

  // assert
  EXPECT_EQ(2, lists.count(Piece::WhiteKnight));
  const auto knights = lists.squares(Piece::WhiteKnight);
  ASSERT_EQ(2U, knights.size());
  EXPECT_EQ((Square::fromStr("b1")), knights[0]);
  EXPECT_EQ((Square::fromStr("g1")), knights[1]);
}

TEST(PieceListsTest, addSeparatesColorsAndKinds) {
  // arrange
  PieceLists lists{};

  // act
  lists.add(Piece::WhitePawn, Square::fromStr("a2"));
  lists.add(Piece::BlackPawn, Square::fromStr("a7"));
  lists.add(Piece::WhiteQueen, Square::fromStr("d1"));

  // assert
  EXPECT_EQ(1, lists.count(Piece::WhitePawn));
  EXPECT_EQ(1, lists.count(Piece::BlackPawn));
  EXPECT_EQ(1, lists.count(Piece::WhiteQueen));
  EXPECT_EQ(0, lists.count(Piece::BlackQueen));
  const auto blackPawns = lists.squares(Piece::BlackPawn);
  ASSERT_EQ(1U, blackPawns.size());
  EXPECT_EQ((Square::fromStr("a7")), blackPawns.front());
}

TEST(PieceListsTest, moveRelocatesOnlyTheMovedPiece) {
  // arrange
  PieceLists lists{};
  lists.add(Piece::WhiteKnight, Square::fromStr("b1"));
  lists.add(Piece::WhiteKnight, Square::fromStr("g1"));

  // act
  lists.move(Piece::WhiteKnight, Square::fromStr("g1"), Square::fromStr("f3"));

  // assert
  EXPECT_EQ(2, lists.count(Piece::WhiteKnight));
  const auto knights = lists.squares(Piece::WhiteKnight);
  ASSERT_EQ(2U, knights.size());
  EXPECT_EQ((Square::fromStr("b1")), knights[0]);
  EXPECT_EQ((Square::fromStr("f3")), knights[1]);
}

TEST(PieceListsTest, removeOnlyPieceEmptiesList) {
  // arrange
  PieceLists lists{};
  lists.add(Piece::BlackRook, Square::fromStr("a8"));

  // act
  lists.remove(Piece::BlackRook, Square::fromStr("a8"));

  // assert
  EXPECT_EQ(0, lists.count(Piece::BlackRook));
  EXPECT_TRUE(lists.squares(Piece::BlackRook).empty());
}

TEST(PieceListsTest, removeLastKeepsRemainingPiecesTracked) {
  // arrange
  PieceLists lists{};
  lists.add(Piece::WhiteKnight, Square::fromStr("b1"));
  lists.add(Piece::WhiteKnight, Square::fromStr("g1"));

  // act
  lists.remove(Piece::WhiteKnight, Square::fromStr("g1"));
  // The survivor must still be movable, which requires its index board
  // entry to be intact.
  lists.move(Piece::WhiteKnight, Square::fromStr("b1"), Square::fromStr("c3"));

  // assert
  const auto knights = lists.squares(Piece::WhiteKnight);
  ASSERT_EQ(1U, knights.size());
  EXPECT_EQ((Square::fromStr("c3")), knights.front());
}

TEST(PieceListsTest, removeNonLastKeepsRemainingPiecesTracked) {
  // arrange
  PieceLists lists{};
  lists.add(Piece::WhiteKnight, Square::fromStr("b1"));
  lists.add(Piece::WhiteKnight, Square::fromStr("g1"));

  // act
  // Removing a non-last entry relocates the last one into the freed slot;
  // the relocated piece must remain movable afterwards.
  lists.remove(Piece::WhiteKnight, Square::fromStr("b1"));
  lists.move(Piece::WhiteKnight, Square::fromStr("g1"), Square::fromStr("e4"));

  // assert
  const auto knights = lists.squares(Piece::WhiteKnight);
  ASSERT_EQ(1U, knights.size());
  EXPECT_EQ((Square::fromStr("e4")), knights.front());
}

TEST(PieceListsTest, captureSequenceReusesSquareAcrossLists) {
  // arrange
  PieceLists lists{};
  lists.add(Piece::WhiteKnight, Square::fromStr("f3"));
  lists.add(Piece::BlackPawn, Square::fromStr("e5"));

  // act
  // The make-capture contract: remove the captured piece first, then move
  // the capturer onto its square. Moving the capturer away again proves
  // the square's index board entry was correctly overwritten.
  lists.remove(Piece::BlackPawn, Square::fromStr("e5"));
  lists.move(Piece::WhiteKnight, Square::fromStr("f3"), Square::fromStr("e5"));
  lists.move(Piece::WhiteKnight, Square::fromStr("e5"), Square::fromStr("d7"));

  // assert
  EXPECT_EQ(0, lists.count(Piece::BlackPawn));
  const auto knights = lists.squares(Piece::WhiteKnight);
  ASSERT_EQ(1U, knights.size());
  EXPECT_EQ((Square::fromStr("d7")), knights.front());
}
