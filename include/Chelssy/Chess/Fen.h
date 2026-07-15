#pragma once

#include "Chelssy/Detail/StringUtils.h"
#include "Consts.h"
#include "Types.h"

#include <array>
#include <cassert>
#include <charconv>
#include <expected>
#include <optional>
#include <string_view>
#include <system_error>
#include <utility>

namespace Chelssy::Chess {

struct Fen {
  /// Enough for any output of toChars(), without a null terminator.
  static constexpr uint8_t strLengthMax = 100;
  static constexpr std::string_view startingPositionStr =
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

  enum class Error : uint8_t {
    InvalidFieldsCount,
    InvalidPiecePlacementValue,
    InvalidSideToMoveValue,
    InvalidCastlingAbilityValue,
    InvalidEnPassantTargetSquareValue,
    InvalidHalfMoveClockValue,
    InvalidFullMoveCounterValue,
  };

  struct PiecePlacement {
    PiecePlacement() = delete;

    [[nodiscard]] static constexpr auto parse(std::string_view str) noexcept
        -> std::expected<PiecePlacement, Error> {
      std::array<Piece, chessboardSize> res{};
      size_t file = 0;
      size_t rank = 0;
      auto hasDigitBefore = false;
      for (const auto chr : str) {
        if (chr == rankDelimiter) {
          if (file != fileMax + 1) {
            return std::unexpected(Error::InvalidPiecePlacementValue);
          }
          hasDigitBefore = false;
          file = 0;
          ++rank;
          if (rank > rankMax) {
            return std::unexpected(Error::InvalidPiecePlacementValue);
          }
          continue;
        }

        if (chr >= '1' && chr <= '8') {
          if (hasDigitBefore) {
            return std::unexpected(Error::InvalidPiecePlacementValue);
          }
          file += static_cast<size_t>(chr - '0');
          hasDigitBefore = true;
          continue;
        }

        if (const auto piece = parsePiece(chr); !isNone(piece)) {
          if (file > fileMax) {
            return std::unexpected(Error::InvalidPiecePlacementValue);
          }
          // 8x8 index
          const auto idx = rank << 3 | file;
          res[idx] = piece;
          ++file;
          hasDigitBefore = false;
          continue;
        }

        return std::unexpected(Error::InvalidPiecePlacementValue);
      }

      if (rank != rankMax || file != fileMax + 1) {
        return std::unexpected(Error::InvalidPiecePlacementValue);
      }

      return PiecePlacement{res};
    }

    [[nodiscard]] static constexpr auto startingPosition() noexcept
        -> PiecePlacement {
      return *parse(startingPositionStr.substr(
          0, startingPositionStr.find(fieldDelimiter)));
    }

    /// @pre sqr.isValid()
    [[nodiscard]] constexpr auto at(const Square sqr) const noexcept -> Piece {
      return inner_[sqr.to8x8() ^ flip8x8RankMask];
    }

    constexpr auto operator==(const PiecePlacement &other) const noexcept
        -> bool = default;

    [[nodiscard]] constexpr auto toChars(char *first,
                                         const char *last) const noexcept
        -> std::to_chars_result {
      uint8_t emptyCnt = 0;
      uint8_t file = 0;
      for (const auto curPiece : inner_) {
        if (file > fileMax) {
          if (emptyCnt > 0) {
            if (!Chelssy::Detail::putChr(first, last,
                                         static_cast<char>(emptyCnt + '0'))) {
              return {.ptr = first, .ec = std::errc::value_too_large};
            }
            emptyCnt = 0;
          }

          if (!Chelssy::Detail::putChr(first, last, rankDelimiter)) {
            return {.ptr = first, .ec = std::errc::value_too_large};
          }
          file = 0;
        }

        if (isNone(curPiece)) {
          ++emptyCnt;
          ++file;
          continue;
        }

        if (emptyCnt > 0) {
          if (!Chelssy::Detail::putChr(first, last,
                                       static_cast<char>(emptyCnt + '0'))) {
            return {.ptr = first, .ec = std::errc::value_too_large};
          }
          emptyCnt = 0;
        }

        if (!Chelssy::Detail::putChr(first, last, piece2FenChar(curPiece))) {
          return {.ptr = first, .ec = std::errc::value_too_large};
        }
        ++file;
      }

      if (emptyCnt > 0) {
        if (!Chelssy::Detail::putChr(first, last,
                                     static_cast<char>(emptyCnt + '0'))) {
          return {.ptr = first, .ec = std::errc::value_too_large};
        }
      }

      return {.ptr = first, .ec = std::errc{}};
    }

  private:
    static constexpr uint8_t flip8x8RankMask = 0x38;
    static constexpr auto rankDelimiter = '/';

    /// @note ranks have flipped order here (index 0 == a8)
    std::array<Piece, chessboardSize> inner_;
    constexpr explicit PiecePlacement(
        const std::array<Piece, chessboardSize> &inner) noexcept
        : inner_(inner) {}

    /// @pre `piece != Piece::None`.
    [[nodiscard]] static constexpr auto
    piece2FenChar(const Piece piece) noexcept -> char {
      assert(!isNone(piece) && "invalid piece");
      constexpr std::array pieceChr = {
          '\0', // we handle Piece::None with a dedicated counter
          'P', 'N', 'B', 'R', 'Q', 'K', // white
          '\0', '\0', // spaces between whites and blacks, not a real Piece
                      // case.
          'p', 'n', 'b', 'r', 'q', 'k' // black
      };
      static_assert(pieceChr[std::to_underlying(Piece::WhitePawn)] == 'P');
      static_assert(pieceChr[std::to_underlying(Piece::WhiteKnight)] == 'N');
      static_assert(pieceChr[std::to_underlying(Piece::WhiteBishop)] == 'B');
      static_assert(pieceChr[std::to_underlying(Piece::WhiteRook)] == 'R');
      static_assert(pieceChr[std::to_underlying(Piece::WhiteQueen)] == 'Q');
      static_assert(pieceChr[std::to_underlying(Piece::WhiteKing)] == 'K');
      static_assert(pieceChr[std::to_underlying(Piece::BlackPawn)] == 'p');
      static_assert(pieceChr[std::to_underlying(Piece::BlackKnight)] == 'n');
      static_assert(pieceChr[std::to_underlying(Piece::BlackBishop)] == 'b');
      static_assert(pieceChr[std::to_underlying(Piece::BlackRook)] == 'r');
      static_assert(pieceChr[std::to_underlying(Piece::BlackQueen)] == 'q');
      static_assert(pieceChr[std::to_underlying(Piece::BlackKing)] == 'k');

      return pieceChr[std::to_underlying(piece)];
    }

    [[nodiscard]] static constexpr auto parsePiece(const char chr) noexcept
        -> Piece {
      switch (chr) {
      case 'P':
        return Piece::WhitePawn;
      case 'N':
        return Piece::WhiteKnight;
      case 'B':
        return Piece::WhiteBishop;
      case 'R':
        return Piece::WhiteRook;
      case 'Q':
        return Piece::WhiteQueen;
      case 'K':
        return Piece::WhiteKing;
      case 'p':
        return Piece::BlackPawn;
      case 'n':
        return Piece::BlackKnight;
      case 'b':
        return Piece::BlackBishop;
      case 'r':
        return Piece::BlackRook;
      case 'q':
        return Piece::BlackQueen;
      case 'k':
        return Piece::BlackKing;
      default:
        return Piece::None;
      }
    }
  };

  Fen() = delete;

private:
  /**
   * `parseField` and `ParseFieldResult` should be defined before `Fen::parse`.
   */

  template <typename F>
  using ParseFieldResult = std::invoke_result_t<F, std::string_view>;

  template <typename F>
  [[nodiscard]] static constexpr auto parseField(std::string_view &str,
                                                 F parser) noexcept
      -> ParseFieldResult<F> {
    auto tok = getFenToken(str);
    if (!tok) {
      return std::unexpected(Error::InvalidFieldsCount);
    }
    return parser(*tok);
  }

public:
  /// @brief Parses a string in FEN notation.
  ///
  /// Validation is purely syntactic: field structure and per-field value
  /// ranges. Position legality (kings present, pawns off the back ranks,
  /// castling rights matching the piece placement, en passant square
  /// matching the side to move, ...) is NOT checked here; that is the
  /// board layer's responsibility.
  ///
  /// Accepts some non-canonical inputs (leading zeros in the counters,
  /// castling flags in any order), while to_chars() always emits the
  /// canonical form. Thus parse(to_chars(f)) == f always holds, but
  /// to_chars(parse(s)) may differ from a non-canonical s.
  ///
  /// @pre fen is trimmed
  [[nodiscard]] static constexpr auto parse(std::string_view fen) noexcept
      -> std::expected<Fen, Error> {
    auto piecePlacement = parseField(fen, PiecePlacement::parse);
    if (!piecePlacement) {
      return std::unexpected(piecePlacement.error());
    }

    auto stm = parseField(fen, parseStm);
    if (!stm) {
      return std::unexpected(stm.error());
    }

    auto castlingAbility = parseField(fen, parseCastlingAbility);
    if (!castlingAbility) {
      return std::unexpected(castlingAbility.error());
    }

    auto epSqr = parseField(fen, parseEpSqr);
    if (!epSqr) {
      return std::unexpected(epSqr.error());
    }

    auto halfMoveClock = parseField(fen, parseHalfMoveClock);
    if (!halfMoveClock) {
      return std::unexpected(halfMoveClock.error());
    }

    auto fullMoveCounter = parseField(fen, parseFullMoveCounter);
    if (!fullMoveCounter) {
      return std::unexpected(fullMoveCounter.error());
    }

    if (!fen.empty()) {
      return std::unexpected(Error::InvalidFieldsCount);
    }

    return Fen{*piecePlacement, *stm,           *castlingAbility,
               *epSqr,          *halfMoveClock, *fullMoveCounter};
  }

  [[nodiscard]] static constexpr auto startingPosition() noexcept -> Fen {
    return *parse(startingPositionStr);
  }

  constexpr auto operator==(const Fen &other) const noexcept -> bool = default;

  /// @brief Writes the position to [first, last) in canonical FEN notation.
  ///
  /// The output is not null-terminated; str_length_max chars are always
  /// sufficient. On success ptr points one past the last written char.
  /// When the buffer is too small, returns errc::value_too_large with the
  /// buffer filled up to the point of failure.
  [[nodiscard]] constexpr auto toChars(char *first, char *last) const noexcept
      -> std::to_chars_result {
    auto res = piecePlacement_.toChars(first, last);
    if (res.ec != std::errc{}) {
      return res;
    }
    first = res.ptr;
    if (!Chelssy::Detail::putChr(first, last, fieldDelimiter)) {
      return {.ptr = first, .ec = std::errc::value_too_large};
    }

    res = stm2FenChars(first, last);
    if (res.ec != std::errc{}) {
      return res;
    }
    first = res.ptr;
    if (!Chelssy::Detail::putChr(first, last, fieldDelimiter)) {
      return {.ptr = first, .ec = std::errc::value_too_large};
    }

    res = castlingAbility2FenChars(first, last);
    if (res.ec != std::errc{}) {
      return res;
    }
    first = res.ptr;
    if (!Chelssy::Detail::putChr(first, last, fieldDelimiter)) {
      return {.ptr = first, .ec = std::errc::value_too_large};
    }

    res = epSqr2FenChars(first, last);
    if (res.ec != std::errc{}) {
      return res;
    }
    first = res.ptr;
    if (!Chelssy::Detail::putChr(first, last, fieldDelimiter)) {
      return {.ptr = first, .ec = std::errc::value_too_large};
    }

    res = std::to_chars(first, last, halfMoveClock_);
    if (res.ec != std::errc{}) {
      return res;
    }
    first = res.ptr;
    if (!Chelssy::Detail::putChr(first, last, fieldDelimiter)) {
      return {.ptr = first, .ec = std::errc::value_too_large};
    }

    res = std::to_chars(first, last, fullMoveCounter_);
    if (res.ec != std::errc{}) {
      return res;
    }
    first = res.ptr;

    return {.ptr = first, .ec = std::errc{}};
  }

  [[nodiscard]] constexpr auto piecePlacement() const noexcept
      -> PiecePlacement {
    return piecePlacement_;
  }

  [[nodiscard]] constexpr auto sideToMove() const noexcept -> Color {
    return sideToMove_;
  }

  [[nodiscard]] constexpr auto castlingAbility() const noexcept
      -> CastlingRights {
    return castlingAbility_;
  }

  [[nodiscard]] constexpr auto enPassantTargetSquare() const noexcept
      -> Square {
    return epSqr_;
  }

  [[nodiscard]] constexpr auto halfMoveClock() const noexcept -> uint8_t {
    return halfMoveClock_;
  }

  [[nodiscard]] constexpr auto fullMoveCounter() const noexcept -> uint16_t {
    return fullMoveCounter_;
  }

private:
  static constexpr auto fieldDelimiter = ' ';
  static constexpr auto noneChr = '-';

  PiecePlacement piecePlacement_;
  Color sideToMove_;
  CastlingRights castlingAbility_;
  Square epSqr_;
  uint8_t halfMoveClock_;
  uint16_t fullMoveCounter_;

  constexpr Fen(const PiecePlacement &piecePlacement, const Color sideToMove,
                const CastlingRights castlingAbility, const Square epSqr,
                const uint8_t halfMoveClock,
                const uint16_t fullMoveCounter) noexcept
      : piecePlacement_(piecePlacement), sideToMove_(sideToMove),
        castlingAbility_(castlingAbility), epSqr_(epSqr),
        halfMoveClock_(halfMoveClock), fullMoveCounter_(fullMoveCounter) {}

  [[nodiscard]] static constexpr auto
  getFenToken(std::string_view &str) noexcept
      -> std::optional<std::string_view> {
    if (str.empty()) {
      return std::nullopt;
    }

    if (const auto tokEnd = str.find(fieldDelimiter);
        tokEnd != std::string_view::npos) {
      if (tokEnd == 0) {
        // we consider consecutive spaces an invalid delimiter.
        return std::nullopt;
      }

      auto tok = str.substr(0, tokEnd);
      str.remove_prefix(tokEnd + 1);
      return tok;
    }

    auto res = str;
    str.remove_prefix(str.length());
    return res;
  }

  [[nodiscard]] static constexpr auto
  parseStm(const std::string_view str) noexcept -> std::expected<Color, Error> {
    if (str.size() != 1) {
      return std::unexpected(Error::InvalidSideToMoveValue);
    }

    switch (str[0]) {
    case 'w':
      return Color::White;
    case 'b':
      return Color::Black;
    default:
      return std::unexpected(Error::InvalidSideToMoveValue);
    }
  }

  [[nodiscard]] static constexpr auto
  parseCastlingAbility(const std::string_view str) noexcept
      -> std::expected<CastlingRights, Error> {
    if (str.empty()) {
      return std::unexpected(Error::InvalidCastlingAbilityValue);
    }

    auto castlingAbility = CastlingRights::None;
    if (str[0] == '-' && str.length() == 1) {
      return castlingAbility;
    }

    for (size_t i = 0; i < str.length(); ++i) {
      CastlingRights cur;
      switch (str[i]) {
      case 'K':
        cur = CastlingRights::WhiteKing;
        break;
      case 'Q':
        cur = CastlingRights::WhiteQueen;
        break;
      case 'k':
        cur = CastlingRights::BlackKing;
        break;
      case 'q':
        cur = CastlingRights::BlackQueen;
        break;
      default:
        return std::unexpected(Error::InvalidCastlingAbilityValue);
      }

      if (hasAny(castlingAbility, cur)) {
        return std::unexpected(Error::InvalidCastlingAbilityValue);
      }

      castlingAbility |= cur;
    }

    return castlingAbility;
  }

  [[nodiscard]] static constexpr auto
  parseEpSqr(const std::string_view str) noexcept
      -> std::expected<Square, Error> {
    if (str == "-") {
      return Square::none();
    }
    if (str.length() != 2 || str[0] < 'a' || str[0] > 'h' ||
        (str[1] != '3' && str[1] != '6')) {
      return std::unexpected(Error::InvalidEnPassantTargetSquareValue);
    }
    return Square::fromStr(str);
  }

  [[nodiscard]] static constexpr auto
  parseHalfMoveClock(const std::string_view str) noexcept
      -> std::expected<uint8_t, Error> {
    uint8_t halfMoveClock;
    const auto [ptr, ec] =
        std::from_chars(str.data(), str.data() + str.length(), halfMoveClock);
    if (ec != std::errc{} || ptr != str.data() + str.length()) {
      return std::unexpected(Error::InvalidHalfMoveClockValue);
    }

    return halfMoveClock;
  }

  [[nodiscard]] static constexpr auto
  parseFullMoveCounter(const std::string_view str) noexcept
      -> std::expected<uint16_t, Error> {
    uint16_t fullMoveCounter;
    const auto [ptr, ec] =
        std::from_chars(str.data(), str.data() + str.length(), fullMoveCounter);

    if (ec != std::errc{} || ptr != str.data() + str.length()) {
      return std::unexpected(Error::InvalidFullMoveCounterValue);
    }

    return fullMoveCounter;
  }

  [[nodiscard]] constexpr auto stm2FenChars(char *first,
                                            const char *last) const noexcept
      -> std::to_chars_result {
    assert(sideToMove_ != Color::End && "unexpected Color sentinel value");
    if (constexpr std::array stmChr = {'w', 'b'}; !Chelssy::Detail::putChr(
            first, last, stmChr[std::to_underlying(sideToMove_)])) {
      return {.ptr = first, .ec = std::errc::value_too_large};
    }
    return {.ptr = first, .ec = std::errc{}};
  }

  [[nodiscard]] constexpr auto
  castlingAbility2FenChars(char *first, const char *last) const noexcept
      -> std::to_chars_result {
    constexpr std::string_view syms = "KQkq";
    static_assert(
        syms[std::countr_zero(std::to_underlying(CastlingRights::WhiteKing))] ==
        'K');
    static_assert(syms[std::countr_zero(
                      std::to_underlying(CastlingRights::WhiteQueen))] == 'Q');
    static_assert(
        syms[std::countr_zero(std::to_underlying(CastlingRights::BlackKing))] ==
        'k');
    static_assert(syms[std::countr_zero(
                      std::to_underlying(CastlingRights::BlackQueen))] == 'q');

    auto hasAnyRights = false;
    for (size_t i = 0; i < syms.size(); ++i) {
      if (const auto cur = static_cast<CastlingRights>(1 << i);
          hasAny(castlingAbility_, cur)) {
        hasAnyRights = true;
        if (!Chelssy::Detail::putChr(first, last, syms[i])) {
          return {.ptr = first, .ec = std::errc::value_too_large};
        }
      }
    }

    if (!hasAnyRights && !Chelssy::Detail::putChr(first, last, noneChr)) {
      return {.ptr = first, .ec = std::errc::value_too_large};
    }

    return {.ptr = first, .ec = std::errc{}};
  }

  [[nodiscard]] constexpr auto epSqr2FenChars(char *first,
                                              const char *last) const noexcept
      -> std::to_chars_result {
    if (!epSqr_.isValid()) {
      if (!Chelssy::Detail::putChr(first, last, noneChr)) {
        return {.ptr = first, .ec = std::errc::value_too_large};
      }
      return {.ptr = first, .ec = std::errc{}};
    }

    if (last - first < 2) {
      return {.ptr = first, .ec = std::errc::value_too_large};
    }

    *first++ = static_cast<char>(epSqr_.file() + 'a');
    *first++ = static_cast<char>(epSqr_.rank() + '1');

    return {.ptr = first, .ec = std::errc{}};
  }
};

} // namespace Chelssy::Chess
