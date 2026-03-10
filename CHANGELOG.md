# Changelog

All notable changes to this library are documented in this file.

## [1.0.0] - 2026-03-11

### Added

- New docs pages: STATUS, ROADMAP, and Register Addressing Guide.
- New integration guides under extras/tools for Modbus Poll workflows (Example 03 and Example 04).
- New examples for atomic integrity and custom function handling.
- Callback Hooks, Core Runtime, and Compile-time Configuration documentation groups.
- Arduino IDE keyword literals for RTU and ASCII.

### Changed

- README promoted to Doxygen main page and reorganized as production entrypoint.
- Doxygen config hardened for production navigation, SVG graphs, and Windows dot path resolution.
- Example set expanded and aligned to field-tested Serial0/Serial1 operational rules.
- Validation documentation consolidated into STATUS with explicit coverage audit.
- RS485 example now demonstrates explicit microsecond DE/RE timing via setTxEnableDelaysUs(10, 10).

### Fixed

- Unknown/custom function fallback now returns standard Modbus Illegal Function exception when callback declines handling.
- Broken/legacy references to removed KNOWN_ISSUES and old manual-testing locations.
- Doxygen unresolved main README reference in architecture docs.

### Removed

- Legacy KNOWN_ISSUES.md in favor of STATUS and ROADMAP.
- Obsolete debug example folder and stale manual-testing source folder.

### Validation

- Doxygen generation succeeds with Graphviz dot diagrams enabled.
- Arduino Mega compile sweep passed for all examples in examples/.
- Arduino Uno compile sweep passed for all examples (local library mode).
