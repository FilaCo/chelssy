# 7. Defer C++20 modules; use headers

Status: Accepted
Date: 2026-07-01

## Context

The project previously moved toward C++20 modules. The target is STM32 built with
`arm-none-eabi-gcc`. Module support across the cross toolchain and CMake is still
fragile, and both ETL and GoogleTest are header-based, so they would be included
rather than imported regardless.

## Decision

Use headers (`.hpp`/`.cpp`) for now. Revisit modules once the cross toolchain is
proven and build times actually hurt. Modularity is enforced by the dependency
direction (ADR 0005), not by the `import` keyword.

## Consequences

- A familiar, portable build.
- Include hygiene remains manual (include guards).
- This supersedes the earlier direction to adopt modules.
