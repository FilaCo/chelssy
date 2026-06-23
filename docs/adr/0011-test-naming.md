# 11. Test naming: identifiers follow code style, labels follow text style

Status: Accepted
Date: 2026-07-08

## Context

Test code contains two different kinds of names. The arguments of `TEST` /
`TEST_P` and the instantiation prefix of `INSTANTIATE_TEST_SUITE_P` are C++
identifiers: GoogleTest concatenates them into class names
(`SuiteName_TestName_Test`), which is why its FAQ forbids underscores there.
Case labels — the `name` fields of `*TestParam` tables used for
`SCOPED_TRACE` and for parameterized value names — are strings read by humans
in test reports; GoogleTest imposes no such restriction on them.

An attempt to use camelBack for both showed that labels lose readability:
they are longer than function names and often contain atoms like square
coordinates, which camelBack mangles (`e4NorthIsE5`), while snake_case keeps
them intact (`e4_north_is_e5`).

## Decision

Name test identifiers by the code rules and case labels by text rules:

- Test suites, fixtures, and instantiation prefixes — `CamelCase`, like
  types (`SquareTest`, `Positions`).
- Test case names — `camelBack`, like functions (`fileRankRoundTrip`).
- String case labels — `snake_case` (`e4_north_is_e5`,
  `starting_position`). Labels that feed parameterized value names must stay
  alphanumeric-plus-underscore; pure `SCOPED_TRACE` labels are free text.

## Consequences

- Generated test names read as `Positions/FenParseTest.parseThenToChars/
  starting_position`: the identifier part matches the code style, the label
  part reads as a sentence.
- The boundary is where code ends and data begins, so the split does not
  contradict the project-wide LLVM naming; no clang-tidy exceptions are
  needed, since string literals are not identifiers.
