# Architecture Decision Records

Each record captures one decision, its context, and its consequences, so that
choices can be revisited deliberately rather than rediscovered. Records are
immutable once accepted; a later record supersedes an earlier one rather than
editing it. Format follows Michael Nygard's template.

| # | Title | Status |
|---|-------|--------|
| [0001](0001-polling-state-machine.md) | Polling state machine for the main loop | Accepted |
| [0002](0002-board-reconciliation.md) | Logical board as source of truth, reconciled to the physical board | Accepted |
| [0003](0003-move-acquisition.md) | Move acquisition contract and Ply legality boundary | Accepted |
| [0004](0004-modes-as-parameters.md) | Game modes as parameters, not separate state machines | Accepted |
| [0005](0005-ports-and-adapters.md) | Hexagonal ports-and-adapters architecture | Accepted |
| [0006](0006-etl-over-std.md) | ETL instead of the C++ standard library | Accepted |
| [0007](0007-defer-cpp20-modules.md) | Defer C++20 modules; use headers | Accepted |
| [0008](0008-project-layout.md) | Project layout with core as a library | Accepted |
| [0009](0009-chess-domain-split.md) | Chess domain split: Types, Board, PlyGen | Accepted |
| [0010](0010-move-generation-strategy.md) | Pseudo-legal move generation with make/unmake | Accepted |
