# 6. ETL instead of the C++ standard library

Status: Accepted
Date: 2026-07-01

## Context

The target is an MCU with no heap and tight RAM. Standard library containers
allocate dynamically, which is unsuitable and non-deterministic on the device.

## Decision

Use the Embedded Template Library (ETL) with static, fixed-capacity allocation
wherever practical, including `etl::fsm` as the state-machine framework.

## Consequences

- Container capacities must be decided up front.
- No dynamic allocation; memory use and timing are deterministic.
- The standard library is still used on the host where appropriate — for example
  in the console adapter and in tests.
