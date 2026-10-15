# Changelog

## 2026-10-15
### Added
- documentation for benchmark module
- documentation for containers module

### Changed
- timer.hpp, was nonsensical.
- BitArray:to_bits_fast to to_bits.
- BitArray:from_bits_fast to from_bits.
- BitArray:to_bits_fast_aligned to to_bits_aligned.
- BitArray:from_bits_fast_aligned to from_bits_aligned.

### Removed
- BitArray::to_bits old implementation
- BitArray::from_bits old implementation

## 2026-10-12
### Added
- functions
- events
- threadpool
- networking

### Changed
- classes on the global library namespace to specific namespaces

### Fixed
- CMakeLists with wrong MPFR variable

## 2026-10-10

### Added
- readme

### Changed
- reworked library structure
- changed pragma once to header guards
- changed integer and floating point type names

## 2026-10-06

### Added
- library itself
