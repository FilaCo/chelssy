#pragma once

#include "Chelssy/Chess/Board.h"
#include "Chelssy/Chess/Consts.h"
#include "Chelssy/Chess/PlyGen.h"
#include "Chelssy/Chess/Types.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>

namespace Chelssy::Chess {

enum class GameState : uint8_t {
  Ongoing = 0,
  WhiteCheckmated,
  BlackCheckmated,
  /* Draw variants */
  Stalemate,
  FiftyMoveDraw,
  Repetition,
  InsufficientMaterial,
};

/// The result a GameState maps to; see gameOutcome.
enum class GameOutcome : uint8_t {
  Ongoing = 0,
  Draw,
  WhiteWins,
  BlackWins,
};

[[nodiscard]] constexpr auto gameState(Board &board) noexcept -> GameState {
  std::array<Ply, pliesPerPositionMax> pliesBuf;
  const auto generated = genPlies<GenKind::All>(board, pliesBuf);

  const auto anyLegal =
      std::ranges::any_of(generated, [&board](const Ply ply) -> bool {
        return isLegal(board, ply);
      });

  using enum GameState;

  if (!anyLegal) {
    constexpr std::array<GameState, colorsCount> checkmated{WhiteCheckmated,
                                                            BlackCheckmated};
    const auto stmIdx = std::to_underlying(board.sideToMove());

    return isInCheck(board) ? checkmated[stmIdx] : Stalemate;
  }

  if (board.plyClock() >= plyClockMax) {
    return FiftyMoveDraw;
  }

  return Ongoing;
}

[[nodiscard]] constexpr auto gameOutcome(const GameState state) noexcept
    -> GameOutcome {
  switch (state) {
  case GameState::Ongoing:
    return GameOutcome::Ongoing;
  case GameState::WhiteCheckmated:
    return GameOutcome::BlackWins;
  case GameState::BlackCheckmated:
    return GameOutcome::WhiteWins;
  case GameState::Stalemate:
  case GameState::FiftyMoveDraw:
  case GameState::Repetition:
  case GameState::InsufficientMaterial:
    return GameOutcome::Draw;
  }
}

static_assert(gameOutcome(GameState::Ongoing) == GameOutcome::Ongoing);
static_assert(gameOutcome(GameState::WhiteCheckmated) ==
              GameOutcome::BlackWins);
static_assert(gameOutcome(GameState::BlackCheckmated) ==
              GameOutcome::WhiteWins);
static_assert(gameOutcome(GameState::Stalemate) == GameOutcome::Draw);
static_assert(gameOutcome(GameState::FiftyMoveDraw) == GameOutcome::Draw);
static_assert(gameOutcome(GameState::Repetition) == GameOutcome::Draw);
static_assert(gameOutcome(GameState::InsufficientMaterial) ==
              GameOutcome::Draw);

} // namespace Chelssy::Chess
