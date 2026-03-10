# Modbus RTU Arduino Library - Developer Context

This library is a professional, industrial-grade Modbus RTU Slave implementation for Arduino, featuring a non-blocking state machine, atomic transaction support, and comprehensive diagnostic capabilities.

## Project Overview

- **Purpose:** Provide a robust, standards-compliant Modbus RTU slave for Arduino-compatible microcontrollers.
- **Main Technologies:** C++, Arduino Framework, Modbus Protocol (RTU).
- **Architecture:** Layered design (Physical -> Core -> Data -> Application) to ensure separation of concerns and scalability.
- **Key Features:**
    - **Non-blocking State Machine:** `slave.run()` ensures no loop stalls, critical for real-time applications.
    - **Atomic Transaction API:** `atomicBegin()`/`atomicEnd()` protect against data races during read-modify-write cycles.
    - **Typed Helpers:** Support for `float` (IEEE-754), `uint32_t`, and `string` data types across register pairs.
    - **Event Callbacks:** `onRead()` and `onWrite()` hooks for just-in-time data sampling and immediate command reaction.
    - **Industrial Robustness:** 5-layer validation chain, CRC-16 integrity checks, and RS485 DE/RE control with microsecond-precision timing.

## Core Components (`src/`)

- `modbus.h`: Core protocol constants, function codes (FC01-06, 15, 16, 08), and compile-time configuration flags.
- `modbusRegBank.h/cpp`: Manages register storage, data access, and atomic locking logic.
- `modbusDevice.h/cpp`: High-level representation of the Modbus slave device (ID management).
- `modbusSlave.h/cpp`: The protocol engine (parser, validation chain, and response builder).

## Building and Running

### Development Environment
- **Standard:** Arduino IDE or PlatformIO.
- **Dependencies:** None (uses standard Arduino `Stream` and `HardwareSerial`).

### Key Commands
- **Documentation:** `doxygen Doxyfile` (Generates HTML API reference in `docs/`).
- **Build/Upload:** Standard Arduino compile/upload flow.
- **Validation:** Use the 5 progressive examples in `examples/` for feature verification.

## Development Conventions

1. **Non-Blocking Logic:** All logic must remain non-blocking. Never use `delay()` in the library core or callbacks.
2. **Memory Management:**
    - Default: `malloc`-backed linked nodes for registers.
    - Static: Enable `MODBUS_USE_STATIC_REG_POOL` in `modbus.h` for no-heap environments.
3. **Data Integrity:** Always use the Atomic Transaction API when performing multi-step operations on registers in the main loop.
4. **Register Addressing:**
    - Coils: `1..2000`
    - Discrete Inputs: `10001..12000`
    - Input Registers: `30001..30125`
    - Holding Registers: `40001..40125`
5. **Coding Style:** Follow existing Arduino/C++ conventions. Use Doxygen-style comments for all public methods and constants.

## Technical Metadata (Compile-time Flags)

| Flag | Default | Description |
| --- | --- | --- |
| `MODBUS_USE_STATIC_REG_POOL` | `0` | Set to `1` to use static memory instead of heap. |
| `MODBUS_MAX_DIG_REGS` | `64` | Max digital registers when static pool is enabled. |
| `MODBUS_MAX_ANA_REGS` | `64` | Max analog registers when static pool is enabled. |
| `MODBUS_MAX_FRAME` | `128` | Max supported RTU frame size (can be increased). |
| `MODBUS_MAX_READ_CALLBACKS` | `8` | Maximum active read callbacks per slave. |
| `MODBUS_MAX_WRITE_CALLBACKS` | `8` | Maximum active write callbacks per slave. |

## Testing Strategy

- **Empirical Validation:** Use a Modbus Master tool (e.g., Modbus Poll, QModMaster) against the provided examples.
- **Progressive Learning:** Start with `01_Basic_MinimalSlave` and move to `05_RS485_Hardware` for final industrial deployment verification.
- **Regression Testing:** Always verify FC15/FC16 multi-write behavior after changes to the data layer.
