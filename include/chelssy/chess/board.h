//
// Created by Dmitrii Chizha on 01.07.2026.
//

#pragma once

#include "types.h"
#include <etl/array.h>

namespace chelssy::chess {

/// Mailbox (0x88) + Piece-Lists chessboard representation.
///
/// @note bitboard is not used here because it requires more RAM to generate
/// moves.
struct board {
  constexpr board() noexcept = default;
  board(const board &) = delete;
  constexpr board(board &&) noexcept = default;

  [[nodiscard]] static auto starting_position() -> board;

  [[nodiscard]] constexpr auto side_to_move() const noexcept -> color {
    return side_to_move_;
  }

private:
  color side_to_move_ = color::white;
  castling_rights castling_rights_ = castling_rights::none;
  square ep_target_sqr_ = square::none();
  /// 0..100
  ///
  /// Resets on moving pawn or taking a piece.
  uint8_t ply_clock_ = 0;
  /// 0..8850
  uint16_t move_counter_ = 1;

  static constexpr uint8_t mailbox_size = 128;
  etl::array<piece, mailbox_size> mailbox_ = {piece::none};

  template <uint8_t MaxN> struct piece_list {
    constexpr piece_list() = default;
    piece_list(const piece_list &) = delete;
    constexpr piece_list(piece_list &&) = default;

  private:
    etl::array<uint8_t, MaxN> squares_;
    uint8_t count_ = 0;
  };

  static constexpr auto color_count = 2;
  //
  // static constexpr uint8_t MaxPawns = 8;
  // etl::array<PieceList<MaxPawns>, ColorSize> Pawns;
  //
  // /// 2 initial + 8 promoted pawns.
  // static constexpr uint8_t MaxKnights = 10;
  // etl::array<PieceList<MaxKnights>, ColorSize> Knights;
  //
  // /// 2 initial + 8 promoted pawns.
  // static constexpr uint8_t MaxBishops = 10;
  // etl::array<PieceList<MaxBishops>, ColorSize> Bishops;
  //
  // /// 2 initial + 8 promoted pawns.
  // static constexpr uint8_t MaxRooks = 10;
  // etl::array<PieceList<MaxRooks>, ColorSize> Rooks;
  //
  // /// 1 initial + 8 promoted pawns.
  // static constexpr uint8_t MaxQueens = 9;
  // etl::array<PieceList<MaxQueens>, ColorSize> Queens;
  //
  // etl::array<uint8_t, ColorSize> Kings;
  //
  // static constexpr uint8_t ChessboardSize = 64;
  // etl::array<etl::array<uint8_t, ChessboardSize>, ColorSize> IndexBoards;
};

} // namespace chelssy::chess
