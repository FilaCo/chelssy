module;

#include "etl/array.h"
#include <cstdint>

export module Chelssy;

export namespace Chelssy::Chess {

enum class Color : uint8_t { WHITE, BLACK, COLOR_END };

class Square {
  static constexpr uint8_t RANK_SHIFT = 4;
  static constexpr uint8_t FILE_MASK = 7;
  uint8_t Inner;

public:
  constexpr Square(const uint8_t Rank, const uint8_t File)
      : Inner((Rank << RANK_SHIFT) | File) {}
  [[nodiscard]] auto rank() const { return Inner >> RANK_SHIFT; }
  [[nodiscard]] auto file() const { return Inner & FILE_MASK; }
  explicit operator size_t() const { return Inner; }
};

enum class Piece : uint8_t {
  NONE,
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING,
  PIECE_END
};

template <uint8_t MaxN> class PieceList {
  etl::array<uint8_t, MaxN> Squares;
  uint8_t Count;
};

// Mailbox (0x88) + Piece-Lists chessboard representation.
// NOTE: bitboard is not used here because it requires more RAM to generate
// moves.
class Chessboard {
  static constexpr uint8_t MAILBOX_SIZE = 128;
  etl::array<Piece, MAILBOX_SIZE> Mailbox;

  Color ColorToMove;

  static constexpr uint8_t MAX_PAWNS = 8;
  etl::array<PieceList<MAX_PAWNS>, static_cast<size_t>(Color::COLOR_END)> Pawns;

  // 2 initial + 8 promoted pawns
  static constexpr uint8_t MAX_KNIGHTS = 10;
  etl::array<PieceList<MAX_KNIGHTS>, static_cast<size_t>(Color::COLOR_END)>
      Knights;

  // 2 initial + 8 promoted pawns
  static constexpr uint8_t MAX_BISHOPS = 10;
  etl::array<PieceList<MAX_BISHOPS>, static_cast<size_t>(Color::COLOR_END)>
      Bishops;

  // 2 initial + 8 promoted pawns
  static constexpr uint8_t MAX_ROOKS = 10;
  etl::array<PieceList<MAX_ROOKS>, static_cast<size_t>(Color::COLOR_END)> Rooks;

  // 1 initial + 8 promoted pawns
  static constexpr uint8_t MAX_QUEENS = 9;
  etl::array<PieceList<MAX_QUEENS>, static_cast<size_t>(Color::COLOR_END)>
      Queens;

  etl::array<uint8_t, static_cast<size_t>(Color::COLOR_END)> Kings;

  static constexpr uint8_t CHESSBOARD_SIZE = 64;
  etl::array<etl::array<uint8_t, CHESSBOARD_SIZE>,
             static_cast<size_t>(Color::COLOR_END)>
      IndexBoards;

public:
  Chessboard(const Chessboard &) = default;
  Chessboard(Chessboard &&) = default;

  [[nodiscard]] static constexpr Chessboard startingPosition() {
    return {.ColorToMove = Color::WHITE};
  }

  [[nodiscard]] auto pieceAt(const Square Sqr) const {
    return Mailbox[static_cast<size_t>(Sqr)];
  }

  [[nodiscard]] auto isEmpty(const Square Sqr) const {
    return pieceAt(Sqr) == Piece::NONE;
  }
  [[nodiscard]] auto isOccupied(const Square Sqr) const {
    return !isEmpty(Sqr);
  }
  [[nodiscard]] auto colorToMove() const { return ColorToMove; }
};

} // namespace Chelssy::Chess
