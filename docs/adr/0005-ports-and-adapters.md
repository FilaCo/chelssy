# 5. Hexagonal ports-and-adapters architecture

Status: Accepted
Date: 2026-07-01

## Context

The same logic must run on the MCU and on a host (console), and it must be unit
testable. Hardware must be swappable without touching the game logic.

## Decision

The system is hexagonal. The dependency direction is the rule that decides where
every file belongs:

    adapters --> ports <-- application --> domain

- The domain (chess: board, `Ply`, rules) depends on nothing.
- The application (the FSM) depends on the domain and on ports, never on a
  concrete adapter.
- Adapters (sensors, motors, console, chess engine) implement the ports and own
  all hardware knowledge.
- The composition root (`entrypoints/`) is the only place that names concrete
  adapters and wires them to the application.

Ports are owned by the application: it declares the interfaces it needs and
hardware conforms. The physical-board port is high-level: `move(From, To)`.

## Consequences

- `host` and `firmware` differ only in which adapters they instantiate; the FSM
  code is identical.
- A test is another composition root wired with fakes. Testability follows
  directly: `tick` is deterministic, all I/O is behind ports (a fake move source
  with scripted `poll()`, a board mover that records commands), and there is no
  wall-clock dependency (a clock, if needed, is a port). A test drives N ticks
  and asserts the state trajectory and the recorded side effects.
