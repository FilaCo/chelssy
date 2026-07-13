#include "Chelssy/Chess/Types.h"
#include <gtest/gtest.h>

using namespace Chelssy::Chess;

TEST(CastlingRightsTest, complementStaysWithinValidFlagSet) {
  // act
  auto rights = CastlingRights::All;
  rights &= ~CastlingRights::WhiteKing;

  // assert
  EXPECT_EQ(CastlingRights::WhiteQueen | CastlingRights::BlackKing |
                CastlingRights::BlackQueen,
            rights);
  EXPECT_EQ(CastlingRights::All, ~CastlingRights::None);
  EXPECT_FALSE(hasAny(rights, CastlingRights::WhiteKing));
}
