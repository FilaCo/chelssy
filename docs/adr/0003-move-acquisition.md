# 3. Move acquisition contract and Ply legality boundary

Status: Accepted
Date: 2026-07-01

## Context

Moves arrive from several sources — a hand on the board, voice, the AI, a replay
script — with different shapes and different validity guarantees. The domain must
not be polluted by unvalidated, source-specific input.

## Decision

A move source is polled through one interface with three outcomes:

    poll() -> Pending | Rejected | Ready(Ply)

`Rejected` (an illegal attempt by a human) is a distinct outcome that transitions
to `Reconciling` for rollback. The AI never returns `Rejected`; it only picks
from legal moves.

`Ply` is legal by construction: it is produced only by a validating factory that
requires the board. Unvalidated input — sensor deltas, a parsed voice command —
is an *intent* that lives in the adapter and never crosses into the domain.

## Consequences

- The state machine only ever sees a `Ply`, which is why `AwaitingPly` is an
  honest name.
- Validation lives at the port boundary: each adapter converts its raw input and
  calls the domain factory.
- The three-outcome poll drives the loop transitions directly (stay, roll back,
  or apply).
