#ifndef CHELSSY_CHESS_HPP
#define CHELSSY_CHESS_HPP

#include "etl/array.h"
#include <cstdint>

namespace Chelssy::Chess {

enum class Color : uint8_t { WHITE, BLACK };

class Square {
  static const uint8_t RANKS_COUNT = 16;
  static const uint8_t FILE_MASK = 7;
  uint8_t Inner;

public:
  Square(uint8_t Rank, uint8_t File) { Inner = (Rank * RANKS_COUNT) + File; }
  [[nodiscard]] auto rank() const { return Inner >> 4; }
  [[nodiscard]] auto file() const { return Inner & FILE_MASK; }
  operator uint8_t() const { return Inner; }
};

enum class Piece : uint8_t { NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

// Mailbox (0x88) + Piece-List chessboard representation.
// NOTE: bitboard is not used here because it requires more RAM to generate
// moves.
class Chessboard {
  static const uint8_t MAILBOX_SIZE = 128;
  etl::array<Piece, MAILBOX_SIZE> Mailbox;

  Color ColorToMove;

  static const uint8_t MAX_PAWNS = 8;
  //
  etl::array<uint8_t, MAX_PAWNS> WhitePawns;
  uint8_t WhitePawnsCount;
  etl::array<uint8_t, MAX_PAWNS> BlackPawns;
  uint8_t BlackPawnsCount;

  // 2 initial + 8 promoted pawns
  static const uint8_t MAX_KNIGHTS = 10;
  etl::array<uint8_t, MAX_KNIGHTS> WhiteKnights;
  uint8_t WhiteKnightsCount;
  etl::array<uint8_t, MAX_KNIGHTS> BlackKnights;
  uint8_t BlackKnightsCount;

  // 2 initial + 8 promoted pawns
  static const uint8_t MAX_BISHOPS = 10;
  etl::array<uint8_t, MAX_BISHOPS> WhiteBishops;
  uint8_t WhiteBishopsCount;
  etl::array<uint8_t, MAX_BISHOPS> BlackBishops;
  uint8_t BlackBishopsCount;

  // 2 initial + 8 promoted pawns
  static const uint8_t MAX_ROOKS = 10;
  etl::array<uint8_t, MAX_ROOKS> WhiteRooks;
  uint8_t WhiteRooksCount;
  etl::array<uint8_t, MAX_ROOKS> BlackRooks;
  uint8_t BlackRooksCount;

  // 1 initial + 8 promoted pawns
  static const uint8_t MAX_QUEENS = 9;
  etl::array<uint8_t, MAX_QUEENS> WhiteQueens;
  uint8_t WhiteQueensCount;
  etl::array<uint8_t, MAX_QUEENS> BlackQueens;
  uint8_t BlackQueensCount;

  uint8_t WhiteKing;
  uint8_t BlackKing;

  static const uint8_t CHESSBOARD_SIZE = 64;
  etl::array<uint8_t, CHESSBOARD_SIZE> WhiteIndexBoard;
  etl::array<uint8_t, CHESSBOARD_SIZE> BlackIndexBoard;

public:
  [[nodiscard]] auto pieceAt(Square Sqr) const { return Mailbox[Sqr]; }

  [[nodiscard]] auto isEmpty(Square Sqr) const {
    return pieceAt(Sqr) == Piece::NONE;
  }
  [[nodiscard]] auto isOccupied(Square Sqr) const { return !isEmpty(Sqr); }
  [[nodiscard]] auto colorToMove() const { return ColorToMove; }
};

} // namespace Chelssy::Chess

#endif // CHELSSY_CHESS_HPP