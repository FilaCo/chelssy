# 12. Contractual encapsulation via Detail namespaces

Status: Accepted
Date: 2026-07-09

## Context

The core library is header-only so that everything stays `constexpr`:
positions parse at compile time, invariants are pinned by `static_assert`,
and the same code paths run under fuzzing. This rules out physical
encapsulation — there is no `.cpp` to hide implementation in, and every
helper a public header uses must itself be reachable from the public
include tree.

Two tensions followed. First, some implementation units have invariants of
their own and deserve standalone white-box tests: `PieceLists` (swap-remove
bookkeeping via index boards) was a private nested class of `Board`,
untestable in isolation — its `remove`/`move` could only be exercised
through a `doPly` that in turn cannot be built safely on untested piece
lists. Second, single-consumer utilities (`putChr`, used only by
`Fen::toChars`) sat in a public-looking header (`Chelssy/Utils/`) with no
stated contract, inviting accidental dependence.

Header-only libraries in the wild (Boost, fmt, Abseil, range-v3) solve this
contractually: implementation lives in `detail`/`internal` namespaces and
directories that are compiled by users but promised to nobody.

## Decision

Adopt the detail convention. Units that are not public API of Chelssy or of
its sublibraries (Chess, App) live under `Detail/` directories and `Detail`
namespaces mirroring their owner:

- `include/Chelssy/Detail/` → `Chelssy::Detail` — cross-library utilities
  (`StringUtils.h`).
- `include/Chelssy/Chess/Detail/` → `Chelssy::Chess::Detail` — internals of
  the chess library (`PieceLists.h`); other sublibraries follow the same
  pattern.

The contract:

- Anything under `Detail` is internal: no stability guarantees, and code
  outside Chelssy must not name it. The directory and namespace themselves
  carry the contract, together with this record; no per-file markers.
- Public signatures of non-detail types never mention detail types
  (`Board::squares` returns `std::span<const Square>`, not a piece list).
- Tests may include detail headers directly; the test tree mirrors the
  include tree (`tests/Core/Chess/Detail/PieceListsTest.cpp`).
- Public headers refer to detail entities by fully qualified name
  (`Chelssy::Detail::putChr`), since a shorter `Detail::` inside
  `Chelssy::Chess` resolves to the sibling `Chess::Detail` first.
- New helpers are born in `Detail`. Promotion to public API happens when a
  second independent consumer appears, together with tests and
  documentation; promotion is painless, while demotion breaks users.

## Consequences

- Encapsulation is by convention, not compiler-enforced: a `Detail` type is
  nameable everywhere it is included. Acceptable for a single-maintainer
  codebase.
- Internal components gain standalone white-box tests: `PieceLists`
  `remove`/`move` can be developed test-first before `doPly` exists.
- Detail tests are allowed to churn when internals change; that is the
  price of white-box testing and does not destabilize the public suite.
- `constexpr` is fully preserved — detail code is still headers, still in
  the public include tree, only excluded from the API contract.
