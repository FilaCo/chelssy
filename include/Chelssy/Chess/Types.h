#pragma once

#include "Consts.h"

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace Chelssy::Chess {

/// Side of play. Also used to tag piece ownership.
enum class Color : uint8_t {
  White = 0,
  Black,
  /// Sentinel value.
  End,
};

static constexpr uint8_t colorsCount = std::to_underlying(Color::End);

[[nodiscard]] constexpr auto flip(const Color color) noexcept -> Color {
  assert(color != Color::End && "invalid color");
  return static_cast<Color>(std::to_underlying(color) ^ 1);
}

static_assert(Color::White == flip(Color::Black));
static_assert(Color::Black == flip(Color::White));

/// Piece kind combined with its color.
///
/// 0..2 bits encode a piece kind.
/// 3 bit encodes a color.
enum class Piece : uint8_t {
  /// Represents empty square.
  None = 0b0000,
  /* White pieces start */
  WhitePawn = 0b0001,
  WhiteKnight = 0b0010,
  WhiteBishop = 0b0011,
  WhiteRook = 0b0100,
  WhiteQueen = 0b0101,
  WhiteKing = 0b0110,
  /* Black pieces start */
  BlackPawn = 0b1001,
  BlackKnight = 0b1010,
  BlackBishop = 0b1011,
  BlackRook = 0b1100,
  BlackQueen = 0b1101,
  BlackKing = 0b1110,
};

enum class PieceKind : uint8_t {
  /// Represents empty square.
  None = 0,
  Pawn,
  Knight,
  Bishop,
  Rook,
  Queen,
  King,
  /// Sentinel value.
  End,
};

/// Dimension for kind-indexed arrays; includes the PieceKind::None slot.
static constexpr uint8_t pieceKindsCount = std::to_underlying(PieceKind::End);
static constexpr uint8_t pieceKindMask = 0x7;

/// Returns piece color.
/// @pre `piece != Piece::None`
[[nodiscard]] constexpr auto getColor(const Piece piece) noexcept -> Color {
  assert(piece != Piece::None && "invalid piece");
  return static_cast<Color>(std::to_underlying(piece) >> 3);
}

static_assert(getColor(Piece::WhitePawn) == Color::White);
static_assert(getColor(Piece::WhiteKnight) == Color::White);
static_assert(getColor(Piece::WhiteBishop) == Color::White);
static_assert(getColor(Piece::WhiteRook) == Color::White);
static_assert(getColor(Piece::WhiteQueen) == Color::White);
static_assert(getColor(Piece::WhiteKing) == Color::White);

static_assert(getColor(Piece::BlackPawn) == Color::Black);
static_assert(getColor(Piece::BlackKnight) == Color::Black);
static_assert(getColor(Piece::BlackBishop) == Color::Black);
static_assert(getColor(Piece::BlackRook) == Color::Black);
static_assert(getColor(Piece::BlackQueen) == Color::Black);
static_assert(getColor(Piece::BlackKing) == Color::Black);

/// Returns piece kind.
/// @pre `piece != Piece::None`
[[nodiscard]] constexpr auto getKind(const Piece piece) noexcept -> PieceKind {
  assert(piece != Piece::None && "invalid piece");
  return static_cast<PieceKind>(std::to_underlying(piece) & pieceKindMask);
}

static_assert(getKind(Piece::WhitePawn) == PieceKind::Pawn);
static_assert(getKind(Piece::WhiteKnight) == PieceKind::Knight);
static_assert(getKind(Piece::WhiteBishop) == PieceKind::Bishop);
static_assert(getKind(Piece::WhiteRook) == PieceKind::Rook);
static_assert(getKind(Piece::WhiteQueen) == PieceKind::Queen);
static_assert(getKind(Piece::WhiteKing) == PieceKind::King);

static_assert(getKind(Piece::BlackPawn) == PieceKind::Pawn);
static_assert(getKind(Piece::BlackKnight) == PieceKind::Knight);
static_assert(getKind(Piece::BlackBishop) == PieceKind::Bishop);
static_assert(getKind(Piece::BlackRook) == PieceKind::Rook);
static_assert(getKind(Piece::BlackQueen) == PieceKind::Queen);
static_assert(getKind(Piece::BlackKing) == PieceKind::King);

/// Makes piece from color and kind.
/// @pre `color != Color::End && kind != PieceKind::None && kind !=
/// PieceKind::End`
[[nodiscard]] constexpr auto makePiece(const Color color, const PieceKind kind)
    -> Piece {
  assert(color != Color::End && "invalid color");
  assert(kind != PieceKind::None && kind != PieceKind::End && "invalid kind");
  return static_cast<Piece>(static_cast<uint8_t>(
      (std::to_underlying(color) << 3) | std::to_underlying(kind)));
}

static_assert(makePiece(Color::White, PieceKind::Pawn) == Piece::WhitePawn);
static_assert(makePiece(Color::White, PieceKind::Knight) == Piece::WhiteKnight);
static_assert(makePiece(Color::White, PieceKind::Bishop) == Piece::WhiteBishop);
static_assert(makePiece(Color::White, PieceKind::Rook) == Piece::WhiteRook);
static_assert(makePiece(Color::White, PieceKind::Queen) == Piece::WhiteQueen);
static_assert(makePiece(Color::White, PieceKind::King) == Piece::WhiteKing);

static_assert(makePiece(Color::Black, PieceKind::Pawn) == Piece::BlackPawn);
static_assert(makePiece(Color::Black, PieceKind::Knight) == Piece::BlackKnight);
static_assert(makePiece(Color::Black, PieceKind::Bishop) == Piece::BlackBishop);
static_assert(makePiece(Color::Black, PieceKind::Rook) == Piece::BlackRook);
static_assert(makePiece(Color::Black, PieceKind::Queen) == Piece::BlackQueen);
static_assert(makePiece(Color::Black, PieceKind::King) == Piece::BlackKing);

/// Castling rights encoded as a single byte.
/// To check if castling is available, do:
/// `hasAny(CastlingRights, CastlingRights::WhiteQueen)`.
///
/// For multiple check use a mask, e.g.:
/// `hasAny(CastlingRights, CastlingRights::BlackKing |
/// CastlingRights::BlackQueen).
enum class CastlingRights : uint8_t {
  /// No castling is available.
  None = 0,
  /// King castle is available for white side.
  WhiteKing = 1 << 0,
  /// Queen castle is available for white side.
  WhiteQueen = 1 << 1,
  /// King castle is available for black side.
  BlackKing = 1 << 2,
  /// Queen castle is available for black side.
  BlackQueen = 1 << 3,
  All = WhiteKing | WhiteQueen | BlackKing | BlackQueen,
  /// Sentinel value.
  End = 1 << 4,
};

[[nodiscard]] constexpr auto operator&(const CastlingRights lhs,
                                       const CastlingRights rhs) noexcept
    -> CastlingRights {
  return static_cast<CastlingRights>(std::to_underlying(lhs) &
                                     std::to_underlying(rhs));
}

[[nodiscard]] constexpr auto operator|(const CastlingRights lhs,
                                       const CastlingRights rhs) noexcept
    -> CastlingRights {
  return static_cast<CastlingRights>(std::to_underlying(lhs) |
                                     std::to_underlying(rhs));
}

/// Complement *within the valid flag set*: unused bits of the byte are
/// never set, so `~x` is always a valid CastlingRights value.
[[nodiscard]] constexpr auto operator~(const CastlingRights val) noexcept
    -> CastlingRights {
  return static_cast<CastlingRights>(~std::to_underlying(val) &
                                     std::to_underlying(CastlingRights::All));
}

constexpr auto operator&=(CastlingRights &lhs,
                          const CastlingRights rhs) noexcept
    -> CastlingRights & {
  lhs = lhs & rhs;
  return lhs;
}

constexpr auto operator|=(CastlingRights &lhs,
                          const CastlingRights rhs) noexcept
    -> CastlingRights & {
  lhs = lhs | rhs;
  return lhs;
}

constexpr auto hasAny(const CastlingRights val,
                      const CastlingRights mask) noexcept -> bool {
  return (val & mask) != CastlingRights::None;
}

static_assert((CastlingRights::All & CastlingRights::WhiteKing) ==
              CastlingRights::WhiteKing);
static_assert((CastlingRights::WhiteKing & CastlingRights::BlackQueen) ==
              CastlingRights::None);

static_assert((CastlingRights::WhiteKing | CastlingRights::WhiteQueen |
               CastlingRights::BlackKing | CastlingRights::BlackQueen) ==
              CastlingRights::All);
static_assert((CastlingRights::None | CastlingRights::BlackKing) ==
              CastlingRights::BlackKing);

static_assert(~CastlingRights::None == CastlingRights::All);
static_assert(~CastlingRights::All == CastlingRights::None);
static_assert(~CastlingRights::WhiteKing ==
              (CastlingRights::WhiteQueen | CastlingRights::BlackKing |
               CastlingRights::BlackQueen));
static_assert(~~CastlingRights::WhiteQueen == CastlingRights::WhiteQueen);

static_assert([]() -> CastlingRights {
  auto rights = CastlingRights::None;
  rights |= CastlingRights::WhiteKing;
  rights |= CastlingRights::BlackQueen;
  return rights;
}() == (CastlingRights::WhiteKing | CastlingRights::BlackQueen));
static_assert([]() -> CastlingRights {
  auto rights = CastlingRights::All;
  rights &= ~CastlingRights::WhiteKing;
  return rights;
}() == (CastlingRights::WhiteQueen | CastlingRights::BlackKing |
        CastlingRights::BlackQueen));

static_assert(hasAny(CastlingRights::All, CastlingRights::WhiteQueen));
static_assert(!hasAny(CastlingRights::None, CastlingRights::All));
static_assert(hasAny(CastlingRights::WhiteKing | CastlingRights::BlackQueen,
                     CastlingRights::BlackKing | CastlingRights::BlackQueen));
static_assert(!hasAny(CastlingRights::WhiteKing,
                      CastlingRights::BlackKing | CastlingRights::BlackQueen));

/// A board square in 0x88 encoding.
///
/// The square index is a single byte laid out as `0rrr_0fff`:
/// - bits 0-2  file (0=a .. 7=h)
/// - bit  3    file-overflow flag
/// - bits 4-6  rank (0=rank 1 .. 7=rank 8)
/// - bit  7    rank-overflow flag
///
/// This is the defining property of 0x88: a square is on the board if
/// none of the overflow bits are set, i.e. `(index & 0x88) == 0`.
struct Square {
  /// Constructs a placeholder square.
  /// @note `isInvalid()`
  constexpr Square() noexcept : Square(Square::none()) {}

  /// @pre `file <= file_max && rank <= rank_max`
  constexpr Square(const uint8_t file, const uint8_t rank) noexcept
      : Square(static_cast<uint8_t>(rank << rankShift) | file) {
    assert(file <= fileMax && rank <= rankMax && "invalid arguments");
  }

  /// Sentinel "no square" value (e.g. no en-passant target).
  ///
  /// @note This is *one* particular invalid square. Invalid squares are not
  /// unique, so `sqr == square::none()` is NOT a validity check - a square
  /// produced by `shifted()` may be invalid yet differ from `none()`. Use
  /// `isValid()` to test validity.
  [[nodiscard]] static constexpr auto none() noexcept -> Square {
    return Square(offBoardMask);
  }

  /// @note Allows to make invalid squares. Check the return value with
  /// `isValid()`.
  [[nodiscard]] static constexpr auto fromIndex(const uint8_t idx) noexcept
      -> Square {
    return Square(idx);
  }

  /// Builds a square from an 8x8 (0..63) index; inverse of `to8x8()`.
  /// @pre `idx < chessboardSize`
  [[nodiscard]] static constexpr auto from8x8(const uint8_t idx) noexcept
      -> Square {
    assert(idx < chessboardSize && "invalid index");
    return Square(static_cast<uint8_t>(idx + (idx & ~fileMask)));
  }

  [[nodiscard]] constexpr auto isValid() const noexcept -> bool {
    return (inner_ & offBoardMask) == 0;
  }

  [[nodiscard]] constexpr auto isInvalid() const noexcept -> bool {
    return !isValid();
  }

  /// @pre `isValid()`; for off-board squares the result is meaningless.
  [[nodiscard]] constexpr auto rank() const noexcept -> uint8_t {
    assert(isValid() && "invalid square");
    return inner_ >> rankShift;
  }
  /// @pre `isValid()`; for off-board squares the result is meaningless.
  [[nodiscard]] constexpr auto file() const noexcept -> uint8_t {
    assert(isValid() && "invalid square");
    return inner_ & fileMask;
  }

  /// @pre `isValid()`; for off-board squares the result is meaningless.
  [[nodiscard]] constexpr auto to8x8() const noexcept -> uint8_t {
    assert(isValid() && "invalid square");
    return (inner_ + (inner_ & fileMask)) >> 1;
  }

  [[nodiscard]] constexpr auto operator==(const Square &other) const noexcept
      -> bool = default;

  /// Raw 0x88 index. `0..0x77` for valid squares; mailbox arrays must be
  /// 128 entries to be indexable by any representable square.
  [[nodiscard]] constexpr auto index() const noexcept -> uint8_t {
    return inner_;
  }

  /// Returns the square displaced by a 0x88 offset: 0x10 is one rank up,
  /// 0x01 one file right (e.g. north-east = 0x11, a knight jump = +-0x21,
  /// +-0x1F, +-0x12, +-0x0E).
  ///
  /// @note Allows to make invalid squares. Check the return value with
  /// `isValid()`.
  [[nodiscard]] constexpr auto shifted(const int8_t offset) const noexcept
      -> Square {
    return fromIndex(static_cast<uint8_t>(inner_ + offset));
  }

private:
  uint8_t inner_ = 0;

  static constexpr uint8_t rankShift = 4;
  static constexpr uint8_t fileMask = 7;
  static constexpr uint8_t offBoardMask = 0x88;

  constexpr explicit Square(const uint8_t inner) noexcept : inner_(inner) {}
};

static_assert(sizeof(Square) == 1);
static_assert(std::is_trivially_copyable_v<Square>);
/// Reference encodings pinning the 0x88 layout.
static_assert(Square(0, 0).index() == 0x00); // a1
static_assert(Square(7, 7).index() == 0x77); // h8
static_assert(Square::none().isInvalid());
static_assert(Square{}.isInvalid());
static_assert(Square::none() == Square{});
static_assert(Square::from8x8(0) == Square(0, 0));                    // a1
static_assert(Square::from8x8(63) == Square(7, 7));                   // h8
static_assert(Square::from8x8(Square(4, 3).to8x8()) == Square(4, 3)); // e4

/// A half-move packed into 16 bits: `ffff_ssssss_tttttt` - 4 bits of
/// Flag, then the from and to squares as 6-bit 8x8 indices.
///
/// Plies live in lists (the generator emits dozens per search ply), so
/// the packed representation halves the search stack footprint.
struct Ply {
  /// What doPly must handle besides moving a piece from `from` to `to`.
  /// A ply carries exactly one Flag; these are codes, not a bitmask.
  ///
  /// Bit 2 marks captures and bit 3 marks promotions with the promoted
  /// kind in bits 0-1; see isCapture/isPromotion/promoKind.
  enum class Flag : uint8_t {
    Quiet = 0b0000,
    DoublePawnPush = 0b0001,
    KingCastle = 0b0010,
    QueenCastle = 0b0011,
    Capture = 0b0100,
    EnPassantCapture = 0b0101,
    KnightPromo = 0b1000,
    BishopPromo = 0b1001,
    RookPromo = 0b1010,
    QueenPromo = 0b1011,
    KnightPromoCapture = 0b1100,
    BishopPromoCapture = 0b1101,
    RookPromoCapture = 0b1110,
    QueenPromoCapture = 0b1111,
  };

  /// @pre `from.isValid() && to.isValid()`
  constexpr Ply(const Square from, const Square to, const Flag flag) noexcept
      : inner_(static_cast<uint16_t>((std::to_underlying(flag) << flagShift) |
                                     (from.to8x8() << fromShift) |
                                     to.to8x8())) {}

  [[nodiscard]] constexpr auto from() const noexcept -> Square {
    return Square::from8x8(
        static_cast<uint8_t>((inner_ >> fromShift) & sqrMask));
  }

  [[nodiscard]] constexpr auto to() const noexcept -> Square {
    return Square::from8x8(static_cast<uint8_t>(inner_ & sqrMask));
  }

  [[nodiscard]] constexpr auto flag() const noexcept -> Flag {
    return static_cast<Flag>(inner_ >> flagShift);
  }

  [[nodiscard]] constexpr auto isCapture() const noexcept -> bool {
    return (inner_ & captureBit) != 0;
  }

  [[nodiscard]] constexpr auto isPromotion() const noexcept -> bool {
    return (inner_ & promoBit) != 0;
  }

  /// @pre `isPromotion()`
  [[nodiscard]] constexpr auto promoKind() const noexcept -> PieceKind {
    assert(isPromotion() && "not a promotion");
    return static_cast<PieceKind>(std::to_underlying(PieceKind::Knight) +
                                  ((inner_ >> flagShift) & promoKindMask));
  }

  [[nodiscard]] constexpr auto operator==(const Ply &other) const noexcept
      -> bool = default;

private:
  uint16_t inner_;

  static constexpr uint8_t fromShift = 6;
  static constexpr uint8_t flagShift = 12;
  static constexpr uint8_t sqrMask = 0b11'1111;
  static constexpr uint8_t promoKindMask = 0b0011;
  static constexpr auto captureBit =
      static_cast<uint16_t>(0b0100U << flagShift);
  static constexpr auto promoBit = static_cast<uint16_t>(0b1000U << flagShift);
};

static_assert(sizeof(Ply) == 2);
static_assert(std::is_trivially_copyable_v<Ply>);

// e2-e4: field extraction round trip.
static_assert(Ply(Square{4, 1}, Square{4, 3}, Ply::Flag::DoublePawnPush)
                  .from() == (Square{4, 1}));
static_assert(Ply(Square{4, 1}, Square{4, 3}, Ply::Flag::DoublePawnPush).to() ==
              (Square{4, 3}));
static_assert(Ply(Square{4, 1}, Square{4, 3}, Ply::Flag::DoublePawnPush)
                  .flag() == Ply::Flag::DoublePawnPush);
static_assert(
    !Ply(Square{4, 1}, Square{4, 3}, Ply::Flag::DoublePawnPush).isCapture());
static_assert(
    !Ply(Square{4, 1}, Square{4, 3}, Ply::Flag::DoublePawnPush).isPromotion());

// Corner-to-corner pins the square packing.
static_assert(Ply(Square{0, 0}, Square{7, 7}, Ply::Flag::Quiet).from() ==
              (Square{0, 0}));
static_assert(Ply(Square{0, 0}, Square{7, 7}, Ply::Flag::Quiet).to() ==
              (Square{7, 7}));

static_assert(Ply(Square{3, 3}, Square{4, 4}, Ply::Flag::Capture).isCapture());
static_assert(
    Ply(Square{4, 4}, Square{3, 5}, Ply::Flag::EnPassantCapture).isCapture());
static_assert(!Ply(Square{4, 4}, Square{3, 5}, Ply::Flag::EnPassantCapture)
                   .isPromotion());

static_assert(Ply(Square{0, 6}, Square{0, 7}, Ply::Flag::KnightPromo)
                  .promoKind() == PieceKind::Knight);
static_assert(Ply(Square{0, 6}, Square{0, 7}, Ply::Flag::BishopPromo)
                  .promoKind() == PieceKind::Bishop);
static_assert(Ply(Square{0, 6}, Square{0, 7}, Ply::Flag::RookPromo)
                  .promoKind() == PieceKind::Rook);
static_assert(Ply(Square{0, 6}, Square{0, 7}, Ply::Flag::QueenPromo)
                  .promoKind() == PieceKind::Queen);
static_assert(
    Ply(Square{0, 6}, Square{0, 7}, Ply::Flag::KnightPromo).isPromotion());
static_assert(
    !Ply(Square{0, 6}, Square{0, 7}, Ply::Flag::KnightPromo).isCapture());

static_assert(
    Ply(Square{0, 6}, Square{1, 7}, Ply::Flag::QueenPromoCapture).isCapture());
static_assert(Ply(Square{0, 6}, Square{1, 7}, Ply::Flag::QueenPromoCapture)
                  .isPromotion());
static_assert(Ply(Square{0, 6}, Square{1, 7}, Ply::Flag::QueenPromoCapture)
                  .promoKind() == PieceKind::Queen);

struct Undo {
  Piece captured;
  CastlingRights castlingRights;
  Square epSqr;
  uint8_t plyClock;
  Undo *prev;
};

} // namespace Chelssy::Chess
