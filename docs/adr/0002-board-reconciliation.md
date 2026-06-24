# 2. Logical board as source of truth, reconciled to the physical board

Status: Accepted
Date: 2026-07-01

## Context

There are two boards: the logical board (game state) and the physical board
(pieces moved by a magnet). They diverge in different directions depending on the
input: an engine or voice move changes the logical board first; a move made by
hand changes the physical board first; an illegal move made by hand leaves the
logical board unchanged. Voice moves mean motors run even in a human/human game,
so reconciliation cannot be special-cased by player type.

## Decision

The logical board is the single source of truth. The `Reconciling` state computes
the difference between the logical and physical boards and drives the magnet to
close only the diverging squares. Its tick is non-blocking: it issues one motor
step and returns `Working`/`Done`.

## Consequences

One mechanism covers three cases without branching on the player:

- engine or voice move: the physical board lags, motors move the pieces;
- legal move by hand: the diff is empty, motors stay idle;
- illegal move by hand: the logical board was untouched, so the diff drives the
  pieces back — rollback is free.

This requires the physical board to be observable (to compute the diff) and a
move primitive to act on it (see ADR 0003 and ADR 0005).
