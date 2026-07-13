# 13. Undo restores an equivalent board, not an identical one

Status: Accepted
Date: 2026-07-11

Supersedes the undo-record clause of ADR 0010 ("undo records that can restore
reordered list entries").

## Context

`PieceLists::remove` is a swap-remove: it reorders the surviving entries.
Undoing a capture or promotion re-adds the removed piece by appending, so the
list order after `doPly` + `undoPly` may differ from the original. ADR 0010
assumed undo records would carry enough information to restore the exact
order; doing so requires storing the removed entry's list index in `Undo` and
an insert-at-index operation in `PieceLists`.

## Decision

`undoPly` guarantees an *equivalent* board: the mailbox, the scalar state
(side to move, castling rights, en-passant square, both counters) and the
piece-list *contents* (as multisets) are restored exactly; the piece-list
order is not.

List order is already an implementation detail: no query exposes it
contractually, and the tests compare piece lists as sets.

## Consequences

- `Undo` stays minimal; no index bookkeeping per search node.
- Board equality checks (tests, reconciliation) must compare observable
  state — mailbox plus scalars — never raw piece-list order.
- Anything that would depend on list order (e.g. a future incremental
  evaluation cache keyed by list slot) is ruled out by this decision and
  would need a superseding record.
