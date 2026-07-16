#pragma once

#include "Consts.h"
#include "Detail/PieceLists.h"
#include "Fen.h"
#include "Types.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <utility>

namespace Chelssy::Chess {

/// Mailbox (0x88) + Piece-Lists chessboard representation.
///
/// @note bitboard is not used here, because Cortex-M has 32 bit architecture,
/// where 64-bit operations are not so cheap.
struct Board {
  enum class Error : uint8_t {
    TooManyPawns,
    TooManyKnights,
    TooManyBishops,
    TooManyRooks,
    TooManyQueens,
    TooManyKings,
    KingMissing,
    PawnOnBackRank,
    InvalidPlyClock,
    InvalidMoveCounter,
    InvalidCastlingRights,
    InvalidEnPassantTargetSquare,
    NonZeroPlyClockWithEnPassant,
    TooManyPromotedPieces,
  };

  Board() = delete;

  /// Builds a board from a syntactically valid FEN, rejecting positions
  /// that break board invariants or static chess legality.
  [[nodiscard]] static constexpr auto fromFen(const Fen &fen) noexcept
      -> std::expected<Board, Error> {
    std::array<Piece, mailboxSize> mailbox{};
    Detail::PieceLists pieceLists;
    if (const auto err =
            fillPlacement(fen.piecePlacement(), mailbox, pieceLists)) {
      return std::unexpected(*err);
    }
    if (const auto err = validatePromotionBudget(pieceLists)) {
      return std::unexpected(*err);
    }
    if (const auto err =
            validateCastlingRights(fen.castlingAbility(), mailbox)) {
      return std::unexpected(*err);
    }
    if (const auto err =
            validateEnPassant(fen.enPassantTargetSquare(), fen.sideToMove(),
                              fen.halfMoveClock(), mailbox)) {
      return std::unexpected(*err);
    }
    if (const auto err =
            validateCounters(fen.halfMoveClock(), fen.fullMoveCounter())) {
      return std::unexpected(*err);
    }

    const auto epSqr =
        normalizeEpSqr(fen.enPassantTargetSquare(), fen.sideToMove(), mailbox);
    return Board{fen.sideToMove(),    fen.castlingAbility(), epSqr,
                 fen.halfMoveClock(), fen.fullMoveCounter(), mailbox,
                 pieceLists};
  }

  [[nodiscard]] constexpr auto sideToMove() const noexcept -> Color {
    return sideToMove_;
  }

  [[nodiscard]] constexpr auto castlingRights() const noexcept
      -> CastlingRights {
    return castlingRights_;
  }

  [[nodiscard]] constexpr auto enPassantTargetSquare() const noexcept
      -> Square {
    return epSqr_;
  }

  [[nodiscard]] constexpr auto plyClock() const noexcept -> uint8_t {
    return plyClock_;
  }

  [[nodiscard]] constexpr auto moveCounter() const noexcept -> uint16_t {
    return moveCounter_;
  }

  [[nodiscard]] constexpr auto pieceAt(const Square sqr) const noexcept
      -> Piece {
    assert(sqr.index() < mailboxSize && "sqr index is out of bounds");
    return mailbox_[sqr.index()];
  }

  [[nodiscard]] constexpr auto isEmpty(const Square sqr) const noexcept
      -> bool {
    return isNone(pieceAt(sqr));
  }

  [[nodiscard]] constexpr auto squares(const Piece piece) const noexcept
      -> std::span<const Square> {
    return pieceLists_.squares(piece);
  }

  /// @pre `color != Color::End`.
  [[nodiscard]] constexpr auto kingSquare(const Color color) const noexcept
      -> Square {
    assert(color != Color::End && "invalid color");
    const auto squares = pieceLists_.squares(makePiece(color, PieceKind::King));
    assert(squares.size() == 1 && "expected exactly one king");
    return squares[0];
  }

  /// Applies a ply and records the irreversible state in `undo`.
  ///
  /// @pre `ply` is pseudo-legal for this position
  /// @note `undo.prev` is untouched.
  /// @note plyClock may exceed plyClockMax during play; draw adjudication
  /// is the caller's job.
  constexpr void doPly(const Ply ply, Undo &undo) noexcept {
    using PlyFlag = Ply::Flag;
    const auto from = ply.from();
    const auto to = ply.to();
    const auto flag = ply.flag();
    const auto mover = pieceAt(from);
    assert(hasColor(mover, sideToMove_) && "no own piece on the from square");

    undo.castlingRights = castlingRights_;
    undo.epSqr = epSqr_;
    undo.plyClock = plyClock_;
    undo.captured = Piece::None;

    // The captured piece leaves first.
    if (ply.isCapture()) {
      const auto capturedSqr = flag == PlyFlag::EnPassantCapture
                                   ? to.shiftedBackwards(sideToMove_)
                                   : to;
      const auto captured = pieceAt(capturedSqr);
      assert(hasColor(captured, flip(sideToMove_)) &&
             "no enemy piece on the capture square");
      assert(getKind(captured) != PieceKind::King && "king capture");
      undo.captured = captured;
      pieceLists_.remove(captured, capturedSqr);
      mailbox_[capturedSqr.index()] = Piece::None;
    }
    assert(isEmpty(to) && "`to` square is occupied");

    mailbox_[from.index()] = Piece::None;
    if (ply.isPromotion()) {
      const auto promoted = makePiece(sideToMove_, ply.promoKind());
      pieceLists_.remove(mover, from);
      pieceLists_.add(promoted, to);
      mailbox_[to.index()] = promoted;
    } else {
      pieceLists_.move(mover, from, to);
      mailbox_[to.index()] = mover;
    }

    if (flag == PlyFlag::KingCastle || flag == PlyFlag::QueenCastle) {
      const auto [rookFrom, rookTo] = castleRookSquares(flag, to.rank());
      const auto rook = makePiece(sideToMove_, PieceKind::Rook);
      assert(pieceAt(rookFrom) == rook && "rook is not on its castling square");
      pieceLists_.move(rook, rookFrom, rookTo);
      mailbox_[rookFrom.index()] = Piece::None;
      mailbox_[rookTo.index()] = rook;
    }

    castlingRights_ &= castlingMasks[from.to8x8()] & castlingMasks[to.to8x8()];
    epSqr_ = flag == PlyFlag::DoublePawnPush &&
                     hasEpCapturer(mailbox_, to, flip(sideToMove_))
                 ? to.shiftedBackwards(sideToMove_)
                 : Square::none();
    plyClock_ = ply.isCapture() || getKind(mover) == PieceKind::Pawn
                    ? 0
                    : static_cast<uint8_t>(plyClock_ + 1);
    if (sideToMove_ == Color::Black) {
      ++moveCounter_;
    }
    sideToMove_ = flip(sideToMove_);
  }

  constexpr void undoPly(const Ply ply, const Undo &undo) noexcept {
    using PlyFlag = Ply::Flag;
    sideToMove_ = flip(sideToMove_);
    if (sideToMove_ == Color::Black) {
      --moveCounter_;
    }
    castlingRights_ = undo.castlingRights;
    epSqr_ = undo.epSqr;
    plyClock_ = undo.plyClock;

    const auto from = ply.from();
    const auto to = ply.to();
    const auto flag = ply.flag();

    if (ply.isPromotion()) {
      const auto promoted = makePiece(sideToMove_, ply.promoKind());
      const auto pawn = makePiece(sideToMove_, PieceKind::Pawn);
      assert(pieceAt(to) == promoted &&
             "promoted piece is not on the to square");
      pieceLists_.remove(promoted, to);
      pieceLists_.add(pawn, from);
      mailbox_[from.index()] = pawn;
    } else {
      const auto mover = pieceAt(to);
      assert(hasColor(mover, sideToMove_) && "no own piece on the to square");
      pieceLists_.move(mover, to, from);
      mailbox_[from.index()] = mover;
    }
    mailbox_[to.index()] = Piece::None;

    if (flag == PlyFlag::KingCastle || flag == PlyFlag::QueenCastle) {
      const auto [rookFrom, rookTo] = castleRookSquares(flag, to.rank());
      const auto rook = makePiece(sideToMove_, PieceKind::Rook);
      assert(pieceAt(rookTo) == rook && "rook is not on its castled square");
      pieceLists_.move(rook, rookTo, rookFrom);
      mailbox_[rookTo.index()] = Piece::None;
      mailbox_[rookFrom.index()] = rook;
    }

    if (!isNone(undo.captured)) {
      const auto capturedSqr = flag == PlyFlag::EnPassantCapture
                                   ? to.shiftedBackwards(sideToMove_)
                                   : to;
      pieceLists_.add(undo.captured, capturedSqr);
      mailbox_[capturedSqr.index()] = undo.captured;
    }
  }

private:
  static constexpr uint16_t moveCounterMin = 1;
  static constexpr uint16_t moveCounterMax = 8850;

  Color sideToMove_;
  CastlingRights castlingRights_;
  /// En-passant target square. Canonical: set only when an enemy pawn
  /// can pseudo-legally capture on it.
  Square epSqr_;
  /// @note 0..plyClockMax
  /// @note Resets on moving pawn or taking a piece.
  uint8_t plyClock_;
  /// @note 1..moveCounterMax
  uint16_t moveCounter_;

  std::array<Piece, mailboxSize> mailbox_;

  // Slot 0 is PieceKind::None; never read, empty squares are skipped
  // before the lookup.
  static constexpr std::array<uint8_t, pieceKindsCount> countMax{0,  8, 10, 10,
                                                                 10, 9, 1};
  static constexpr std::array<uint8_t, pieceKindsCount> initialCount{0, 8, 2, 2,
                                                                     2, 1, 1};
  static constexpr std::array<Error, pieceKindsCount> tooManyError{
      Error::TooManyPawns,   Error::TooManyPawns, Error::TooManyKnights,
      Error::TooManyBishops, Error::TooManyRooks, Error::TooManyQueens,
      Error::TooManyKings};

  using CastlingMasks = std::array<CastlingRights, chessboardSize>;
  static constexpr auto castlingMasks = []() -> CastlingMasks {
    CastlingMasks masks{};
    masks.fill(CastlingRights::All);
    masks[Square::fromStr("a1").to8x8()] = ~CastlingRights::WhiteQueen;
    masks[Square::fromStr("h1").to8x8()] = ~CastlingRights::WhiteKing;
    masks[Square::fromStr("e1").to8x8()] =
        ~(CastlingRights::WhiteKing | CastlingRights::WhiteQueen);
    masks[Square::fromStr("a8").to8x8()] = ~CastlingRights::BlackQueen;
    masks[Square::fromStr("h8").to8x8()] = ~CastlingRights::BlackKing;
    masks[Square::fromStr("e8").to8x8()] =
        ~(CastlingRights::BlackKing | CastlingRights::BlackQueen);
    return masks;
  }();

  struct RookMove {
    Square from;
    Square to;
  };

  /// @pre `flag` is KingCastle or QueenCastle.
  [[nodiscard]] static constexpr auto
  castleRookSquares(const Ply::Flag flag, const uint8_t rank) noexcept
      -> RookMove {
    assert((flag == Ply::Flag::KingCastle || flag == Ply::Flag::QueenCastle) &&
           "not a castle");
    return flag == Ply::Flag::KingCastle
               ? RookMove{.from = Square{7, rank}, .to = Square{5, rank}}
               : RookMove{.from = Square{0, rank}, .to = Square{3, rank}};
  }

  Detail::PieceLists pieceLists_;

  constexpr Board(const Color sideToMove, const CastlingRights castlingRights,
                  const Square epSqr, const uint8_t plyClock,
                  const uint16_t moveCounter,
                  const std::array<Piece, mailboxSize> &mailbox,
                  const Detail::PieceLists &pieceLists) noexcept
      : sideToMove_(sideToMove), castlingRights_(castlingRights), epSqr_(epSqr),
        plyClock_(plyClock), moveCounter_(moveCounter), mailbox_(mailbox),
        pieceLists_(pieceLists) {}

  [[nodiscard]] static constexpr auto
  fillPlacement(const Fen::PiecePlacement &placement,
                std::array<Piece, mailboxSize> &mailbox,
                Detail::PieceLists &pieceLists) noexcept
      -> std::optional<Error> {
    for (size_t idx = 0; idx < mailboxSize; ++idx) {
      const auto sqr = Square::fromIndex(static_cast<uint8_t>(idx));
      if (!sqr.isValid()) {
        continue;
      }
      const auto piece = placement.at(sqr);
      if (isNone(piece)) {
        continue;
      }
      const auto kindIdx = std::to_underlying(getKind(piece));
      if (pieceLists.count(piece) >= countMax[kindIdx]) {
        return tooManyError[kindIdx];
      }
      // No pawn of either color can stand on a back rank: it either
      // could not have moved there or must have promoted.
      if (getKind(piece) == PieceKind::Pawn &&
          (sqr.rank() == 0 || sqr.rank() == rankMax)) {
        return Error::PawnOnBackRank;
      }

      pieceLists.add(piece, sqr);
      mailbox[idx] = piece;
    }
    if (pieceLists.count(Piece::WhiteKing) == 0 ||
        pieceLists.count(Piece::BlackKing) == 0) {
      return Error::KingMissing;
    }

    return std::nullopt;
  }

  [[nodiscard]] static constexpr auto
  validatePromotionBudget(const Detail::PieceLists &pieceLists) noexcept
      -> std::optional<Error> {
    for (uint8_t colorIdx = 0; colorIdx < colorsCount; ++colorIdx) {
      const auto color = static_cast<Color>(colorIdx);
      uint8_t budget = pieceLists.count(makePiece(color, PieceKind::Pawn));
      for (auto kindIdx = std::to_underlying(PieceKind::Knight);
           kindIdx <= std::to_underlying(PieceKind::Queen); ++kindIdx) {
        const auto count =
            pieceLists.count(makePiece(color, static_cast<PieceKind>(kindIdx)));
        if (count > initialCount[kindIdx]) {
          budget = static_cast<uint8_t>(budget + count - initialCount[kindIdx]);
        }
      }
      if (budget > initialCount[std::to_underlying(PieceKind::Pawn)]) {
        return Error::TooManyPromotedPieces;
      }
    }

    return std::nullopt;
  }

  [[nodiscard]] static constexpr auto
  validateCastlingRights(const CastlingRights rights,
                         const std::array<Piece, mailboxSize> &mailbox) noexcept
      -> std::optional<Error> {
    struct Requirement {
      CastlingRights right;
      Square kingSqr;
      Square rookSqr;
      Piece king;
      Piece rook;
    };
    constexpr std::array requirements{
        Requirement{.right = CastlingRights::WhiteKing,
                    .kingSqr = Square::fromStr("e1"),
                    .rookSqr = Square::fromStr("h1"),
                    .king = Piece::WhiteKing,
                    .rook = Piece::WhiteRook},
        Requirement{.right = CastlingRights::WhiteQueen,
                    .kingSqr = Square::fromStr("e1"),
                    .rookSqr = Square::fromStr("a1"),
                    .king = Piece::WhiteKing,
                    .rook = Piece::WhiteRook},
        Requirement{.right = CastlingRights::BlackKing,
                    .kingSqr = Square::fromStr("e8"),
                    .rookSqr = Square::fromStr("h8"),
                    .king = Piece::BlackKing,
                    .rook = Piece::BlackRook},
        Requirement{.right = CastlingRights::BlackQueen,
                    .kingSqr = Square::fromStr("e8"),
                    .rookSqr = Square::fromStr("a8"),
                    .king = Piece::BlackKing,
                    .rook = Piece::BlackRook},
    };

    for (const auto &req : requirements) {
      if (!hasAny(rights, req.right)) {
        continue;
      }
      if (mailbox[req.kingSqr.index()] != req.king ||
          mailbox[req.rookSqr.index()] != req.rook) {
        return Error::InvalidCastlingRights;
      }
    }

    return std::nullopt;
  }

  /// Returns true when an en-passant capture is
  /// pseudo-legally available.
  [[nodiscard]] static constexpr auto
  hasEpCapturer(const std::array<Piece, mailboxSize> &mailbox,
                const Square pawnSqr, const Color capturer) noexcept -> bool {
    constexpr std::array offsets{Square::east, Square::west};
    const auto pawn = makePiece(capturer, PieceKind::Pawn);
    return std::ranges::any_of(offsets, [&](const int8_t offset) -> bool {
      const auto sqr = pawnSqr.shifted(offset);
      return sqr.isValid() && mailbox[sqr.index()] == pawn;
    });
  }

  /// Clears an en-passant square no enemy pawn can capture on (X-FEN
  /// convention), keeping `epSqr_` canonical.
  [[nodiscard]] static constexpr auto
  normalizeEpSqr(const Square epSqr, const Color sideToMove,
                 const std::array<Piece, mailboxSize> &mailbox) noexcept
      -> Square {
    if (epSqr.isInvalid()) {
      return Square::none();
    }
    const auto pawnSqr = epSqr.shiftedBackwards(sideToMove);
    return hasEpCapturer(mailbox, pawnSqr, sideToMove) ? epSqr : Square::none();
  }

  [[nodiscard]] static constexpr auto
  validateEnPassant(const Square epSqr, const Color sideToMove,
                    const uint8_t plyClock,
                    const std::array<Piece, mailboxSize> &mailbox) noexcept
      -> std::optional<Error> {
    if (epSqr.isInvalid()) {
      return std::nullopt;
    }

    if (plyClock != 0) {
      return Error::NonZeroPlyClockWithEnPassant;
    }

    if (epSqr.rank() != relativeRank(sideToMove, rankMax - 2)) {
      return Error::InvalidEnPassantTargetSquare;
    }

    // The enemy pawn double-pushed through epSqr, so it now stands one
    // step "behind" epSqr from the side to move's perspective.
    const auto pawnSqr = epSqr.shiftedBackwards(sideToMove);
    const auto originSqr = epSqr.shiftedForwards(sideToMove);
    const auto pawn = makePiece(flip(sideToMove), PieceKind::Pawn);
    if (!isNone(mailbox[epSqr.index()]) ||
        !isNone(mailbox[originSqr.index()]) ||
        mailbox[pawnSqr.index()] != pawn) {
      return Error::InvalidEnPassantTargetSquare;
    }

    return std::nullopt;
  }

  [[nodiscard]] static constexpr auto
  validateCounters(const uint8_t plyClock, const uint16_t moveCounter) noexcept
      -> std::optional<Error> {
    if (plyClock > plyClockMax) {
      return Error::InvalidPlyClock;
    }
    if (moveCounter < moveCounterMin || moveCounter > moveCounterMax) {
      return Error::InvalidMoveCounter;
    }

    return std::nullopt;
  }
};

} // namespace Chelssy::Chess
