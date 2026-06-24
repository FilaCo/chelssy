# 4. Game modes as parameters, not separate state machines

Status: Accepted
Date: 2026-07-01

## Context

The board supports several modes: human/human, human/AI, AI/AI, solving studies,
replaying games, and eventually Chess960. The question was whether each mode
needs its own state machine.

## Decision

Modes are parameters and injected strategies, not separate machines. What a mode
changes is configuration: which move sources feed each side, the starting
position, the end-of-game predicate, and the rule variant. The loop topology is
identical across modes. Chess960 changes the castling rules — a move-generator
concern (see ADR 0010) — not the loop, so it is not a separate machine either.

## Consequences

- A single code path: no duplication, and a smaller firmware binary.
- Modes are selected and wired at the composition root (see ADR 0005).
- A future mode that genuinely needs different states would be an exception,
  handled when it arises rather than anticipated now.
