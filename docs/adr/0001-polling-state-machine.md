# 1. Polling state machine for the main loop

Status: Accepted
Date: 2026-07-01

## Context

The firmware runs a superloop on an MCU. While waiting for a move it must still
service other work — display, watchdog, motor stepping. It must also be unit
testable. A blocking loop never yields control: it cannot be stepped from a test,
and it starves everything else while it waits.

## Decision

The main loop is a non-blocking polling finite state machine. Every step is a
fast `tick` that returns the next state and yields.

States are situations the machine rests in; transitions are actions it performs.
The distinction is enforced by naming: states are nouns or gerunds, transitions
are verbs. A step that runs to completion within a single tick is a transition,
not a state.

- States: `Idle` -> `Reconciling` -> `AwaitingPly` -> `GameOver`
- Transitions (not states): `acquire`, `validate`, `apply`, `checkEnd`

## Consequences

- `tick` must never block or read wall-clock time.
- Long operations (motor motion, waiting for input) are modeled as states that
  return a progress result (`Working`/`Pending`) and are re-entered each loop.
- The design enables deterministic, step-by-step testing (see ADR 0005).
- It costs slightly more bookkeeping than a blocking loop.
