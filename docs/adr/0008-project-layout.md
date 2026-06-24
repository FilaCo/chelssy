# 8. Project layout with core as a library

Status: Accepted
Date: 2026-07-01

## Context

Several targets — firmware, host, and tests — consume the same core logic. The
layout should make the ports-and-adapters boundaries (ADR 0005) visible and keep
the reused core cleanly separated from hardware.

## Decision

`core` (chess + app + ports) is a hardware-free library that compiles for both
host and target, using the `include/` vs `src/` split. Adapters and composition
roots are terminal consumers, so their headers and sources are co-located rather
than split.

    chelssy/
    ├── include/Chelssy/
    │   ├── Chess/          # domain: types, board, move generation
    │   ├── App/            # application: etl::fsm states and events
    │   └── Ports/          # interfaces: MoveSource, BoardMover, Clock, Display
    ├── src/
    │   ├── Chess/
    │   └── App/
    ├── adapters/
    │   ├── stm32/          # real hardware
    │   ├── console/        # host: stdin source, stdout mover
    │   └── engine/         # AI as a move source
    ├── entrypoints/
    │   ├── firmware/       # composition root for the MCU
    │   └── host/           # composition root for the PC
    └── test/               # GoogleTest, with fakes/

CMake targets: `chelssy-core` (linked into everything, including tests), adapter
libraries per family, one executable per composition root, and a test executable
built from `core` plus fakes.

## Consequences

- The reused core has a clear public surface; the `include/` vs `src/` split is
  justified only for it.
- Terminal components stay simple with co-located sources.
