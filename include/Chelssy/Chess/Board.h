#pragma once

#include "Chelssy/Chess/Consts.h"
#include "Chelssy/Chess/Detail/PieceLists.h"
#include "Fen.h"
#include "Types.h"
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
    WhitePawnOnFirstRank,
    BlackPawnOnLastRank,
    InvalidPlyClock,
    InvalidMoveCounter,
    InvalidCastlingRights,
    InvalidEnPassantTargetSquare,
    NonZeroPlyClockWithEnPassant,
  };

  Board() = delete;

  /// Builds a board from a syntactically valid FEN, rejecting positions
  /// that break board invariants or static chess legality; the validation
  /// helpers below document the exact rules.
  ///
  /// @note Checks that require attack detection (the side not to move must
  /// not be in check) are deferred until the ply generator lands.
  [[nodiscard]] static constexpr auto fromFen(const Fen &fen) noexcept
      -> std::expected<Board, Error> {
    std::array<Piece, mailboxSize> mailbox{};
    Detail::PieceLists pieceLists;
    if (const auto err =
            fillPlacement(fen.piecePlacement(), mailbox, pieceLists)) {
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

    return Board{
        fen.sideToMove(),    fen.castlingAbility(), fen.enPassantTargetSquare(),
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

  /// Plies since the last capture or pawn move (the fifty-move rule
  /// counter).
  [[nodiscard]] constexpr auto plyClock() const noexcept -> uint8_t {
    return plyClock_;
  }

  /// Full move counter; starts at 1, increments after each black ply.
  [[nodiscard]] constexpr auto moveCounter() const noexcept -> uint16_t {
    return moveCounter_;
  }

  [[nodiscard]] constexpr auto pieceAt(const Square sqr) const noexcept
      -> Piece {
    return mailbox_[sqr.index()];
  }

  [[nodiscard]] constexpr auto squares(const Color color,
                                       const PieceKind kind) const noexcept
      -> std::span<const Square> {
    return pieceLists_.squares(color, kind);
  }

  // constexpr void doPly(const Ply ply, Undo &undo) noexcept {}

  // constexpr void undoPly(const Ply ply, const Undo &undo) noexcept {}

private:
  Color sideToMove_;
  CastlingRights castlingRights_;
  /// En-passant target square.
  Square epSqr_;
  /// @note 0..PlyClockMax
  /// @note Resets on moving pawn or taking a piece.
  uint8_t plyClock_;
  /// @note 0..MoveCounterMax
  uint16_t moveCounter_;

  static constexpr uint8_t mailboxSize = 128;
  std::array<Piece, mailboxSize> mailbox_;

  // Slot 0 is PieceKind::None; never read, empty squares are skipped
  // before the lookup.
  static constexpr std::array<uint8_t, pieceKindsCount> countMax{0,  8, 10, 10,
                                                                 10, 9, 1};
  static constexpr std::array<Error, pieceKindsCount> tooManyError{
      Error::TooManyPawns,   Error::TooManyPawns, Error::TooManyKnights,
      Error::TooManyBishops, Error::TooManyRooks, Error::TooManyQueens,
      Error::TooManyKings};

  Detail::PieceLists pieceLists_;

  constexpr Board(const Color sideToMove, const CastlingRights castlingRights,
                  const Square epSqr, const uint8_t plyClock,
                  const uint16_t moveCounter,
                  const std::array<Piece, mailboxSize> &mailbox,
                  const Detail::PieceLists &pieceLists) noexcept
      : sideToMove_(sideToMove), castlingRights_(castlingRights), epSqr_(epSqr),
        plyClock_(plyClock), moveCounter_(moveCounter), mailbox_(mailbox),
        pieceLists_(pieceLists) {}

  /// Scans the placement into the mailbox and the piece lists, enforcing
  /// per-kind count limits, pawn rank restrictions and king presence.
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
      if (piece == Piece::None) {
        continue;
      }
      const auto kindIdx = std::to_underlying(getKind(piece));
      if (pieceLists.count(piece) >= countMax[kindIdx]) {
        return tooManyError[kindIdx];
      }
      if (piece == Piece::WhitePawn && sqr.rank() == 0) {
        return Error::WhitePawnOnFirstRank;
      }
      if (piece == Piece::BlackPawn && sqr.rank() == rankMax) {
        return Error::BlackPawnOnLastRank;
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
                    .kingSqr = Square{4, 0},
                    .rookSqr = Square{7, 0},
                    .king = Piece::WhiteKing,
                    .rook = Piece::WhiteRook},
        Requirement{.right = CastlingRights::WhiteQueen,
                    .kingSqr = Square{4, 0},
                    .rookSqr = Square{0, 0},
                    .king = Piece::WhiteKing,
                    .rook = Piece::WhiteRook},
        Requirement{.right = CastlingRights::BlackKing,
                    .kingSqr = Square{4, 7},
                    .rookSqr = Square{7, 7},
                    .king = Piece::BlackKing,
                    .rook = Piece::BlackRook},
        Requirement{.right = CastlingRights::BlackQueen,
                    .kingSqr = Square{4, 7},
                    .rookSqr = Square{0, 7},
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

    const auto whiteToMove = sideToMove == Color::White;
    const uint8_t expectedRank = whiteToMove ? rankMax - 2 : 2;
    if (epSqr.rank() != expectedRank) {
      return Error::InvalidEnPassantTargetSquare;
    }

    const int8_t towardsPawn = whiteToMove ? -0x10 : 0x10;
    const auto pawnSqr = epSqr.shifted(towardsPawn);
    const auto originSqr = epSqr.shifted(static_cast<int8_t>(-towardsPawn));
    const auto pawn = whiteToMove ? Piece::BlackPawn : Piece::WhitePawn;
    if (mailbox[epSqr.index()] != Piece::None ||
        mailbox[originSqr.index()] != Piece::None ||
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
