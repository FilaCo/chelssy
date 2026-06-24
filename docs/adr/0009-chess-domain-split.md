# 9. Chess domain split: Types, Board, PlyGen

Status: Accepted
Date: 2026-07-01

## Context

The Chess Programming Wiki recommends separating board representation from move
generation. The current `Chessboard` mixes representation with rules: it exposes
`isLegal` and `isGameOver`, which need move generation to answer.

## Decision

Split the chess domain into three components with a one-way dependency:

- Types — `Square`, `Piece`, `Color`, `Ply`. The shared vocabulary; depends on
  nothing.
- Board (`Chessboard`) — storage (0x88 mailbox and piece lists), queries
  (`pieceAt`, `isEmpty`, `sideToMove`, castling rights, en passant), and move
  application. Knows no rules.
- PlyGen — move generation, `isLegal`, and `isGameOver`. Depends on Board.

The dependency runs one way, PlyGen -> Board. The two communicate through `Ply`,
a rich value type carrying flags (capture, castling, en passant, promotion):
PlyGen knows the rules and sets the flags; Board reads them and mutates
mechanically (castling also moves the rook, en passant removes the captured pawn,
promotion swaps the piece), without any rule knowledge. The component is named
`PlyGen` rather than the conventional `MoveGen` to stay consistent with the `Ply`
type; a doc-comment points to the wiki's "Move Generation" for the mapping.

## Consequences

- The representation can be tested and swapped independently of the rules.
- The current `Chessboard` is reshaped: `isLegal` and `isGameOver` move into
  PlyGen, and `Ply` moves into Types and gains its flags.
