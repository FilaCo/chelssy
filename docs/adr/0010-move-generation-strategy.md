# 10. Pseudo-legal move generation with make/unmake

Status: Accepted
Date: 2026-07-01

## Context

Move generation targets an MCU (limited flash, tight RAM, modest CPU) and feeds an
AI mode that searches a game tree, so generation runs in a hot loop. The choice is
between generating fully-legal moves directly and generating pseudo-legal moves
then filtering for legality.

## Decision

Generate pseudo-legal moves with make/unmake, and determine legality by attack
detection. Reasons, given the target:

- The simplest generator means less flash and fewer bugs — no pin or
  check-evasion logic in the generator itself.
- Tree search needs make/unmake regardless, so the machinery is reused rather
  than added; the usual argument for fully-legal generation (avoiding
  make/unmake) does not apply here.
- RAM cost is small: a depth-bounded stack of small undo records. Copy-make is
  rejected — the board is several hundred bytes, and copying it per search node
  would overflow the stack.
- The piece-list representation requires undo records that can restore reordered
  list entries (a swap-remove changes list order).

## Consequences

- The Board move is reversible: `doPly(Ply, Undo &)` records the irreversible
  state (castling rights, en-passant square, ply clock, captured piece) into a
  caller-provided record, and `undoPly(Ply, const Undo &)` restores it. The
  caller owns the undo records, so search keeps them in its own depth-bounded
  stack instead of the board allocating anything.

## Future work

A hybrid legality check — using pins and attack detection to validate a
pseudo-legal move without a full make/unmake — can regain most of the speed of
fully-legal generation without the generator complexity. Introduce it only when
AI search speed becomes the bottleneck.
