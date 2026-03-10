# Modbus RTU Arduino Library

@mainpage Modbus RTU Arduino Library

Standard Modbus RTU slave library for Arduino with support for core Modbus function codes and flexible register-bank usage.

## Field Rules (Load-Bearing)

- Opening a USB COM port typically resets Arduino boards.
- Wait about 2 to 3 seconds after COM open before the first Modbus request.
- If Modbus uses Serial, keep Serial Monitor closed and avoid Serial.print output.
- On single-UART boards, USB Modbus traffic and USB debug logs cannot run reliably at the same time.
- On multi-UART boards (for example MEGA), move Modbus to Serial1/Serial2/Serial3 when you need USB debug logs.

## System Block Diagram

@dot
digraph MainBlock {
  rankdir=LR;
  node [shape=box, style=rounded, fontname=Helvetica, fontsize=10];

  Master [label="Modbus Master\n(PLC/SCADA/PC)", fillcolor="#f2f2f2", style="rounded,filled"];
  RS485 [label="RS485 Transceiver\n(MAX485/MAX3485)", fillcolor="#e6f2ff", style="rounded,filled"];
  Slave [label="Arduino\n(modbusSlave)", fillcolor="#e6ffe6", style="rounded,filled"];
  Regs [label="Register Bank\n(modbusRegBank)", fillcolor="#fff5e6", style="rounded,filled"];

  Master -> RS485 [label="RTU/ASCII Frames"];
  RS485 -> Slave [label="UART bytes"];
  Slave -> Regs [label="Read/Write + callbacks"];
  Regs -> Slave [label="Response data"];
  Slave -> RS485 [label="Response frame"];
  RS485 -> Master [label="Bus reply"];
}
@enddot

## 5-Line Quick Start

```cpp
modbusDevice regBank; modbusSlave slave;
regBank.setId(1); regBank.add(40001); regBank.set(40001, 1234);
slave.setDevice(&regBank); slave.setPort(Serial);
slave.setProtocol(RTU); slave.setBaud(9600);
void loop() { slave.run(); }
```

## Version 1.0.0

**Recent Improvements:**

- ✅ Fixed CRC table linker errors (moved from .h to .cpp)
- ✅ Fixed broken example code to use correct API
- ✅ Added comprehensive inline documentation
- ✅ Published STATUS.md with validation scope and production readiness notes
- ✅ Added FC15/FC16 multi-write support
- ✅ Added `Stream` transport support
- ✅ Added microsecond-based RTU frame timing (3.5 char silence)
- ✅ Added optional static register pools (no-heap mode)
- ✅ Added `keywords.txt` for Arduino IDE highlighting
- ✅ Added non-blocking byte-wise parser state machine
- ✅ Added 32-bit endianness configuration (`configureEndianness`)
- ✅ Added unknown/custom function hook (`onUnknownFunction`)
- ✅ **Added Atomic Transaction API** - `atomicBegin()`, `atomicEnd()`, `atomicGet()`, `atomicSet()` protect against data races during read-modify-write operations in complex loop logic (industrial-grade feature)
- ✅ **Professional Doxygen Documentation** - Complete HTML API reference with module groups, searchable function docs, and class diagrams

## Quick Start

1. **Install:** Add the `.zip` file via Arduino IDE → Sketch → Include Library → Add .ZIP Library
2. **Start with Basic Example:** Open `01_Basic_MinimalSlave` from Examples menu
3. **Wire your hardware:** Connect RX/TX to your Modbus master (or test via USB)
4. **Configure:** Set slave ID and baud rate (defaults: ID=1, baud=9600)
5. **Upload and test** with a Modbus master tool (Modbus Poll, QModMaster, pymodbus, etc.)

See [STATUS.md](STATUS.md) for tested-vs-untested validation scope, [ROADMAP.md](ROADMAP.md) for next-version goals, and [CHANGELOG.md](CHANGELOG.md) for release notes.

## Integration Guides

- @subpage register_addressing_guide
- @subpage validation
- @subpage modbus_poll_example03
- @subpage modbus_poll_example04

## Doxygen Documentation

This repository now includes a ready-to-run `Doxyfile`.

### Generate HTML Docs

1. **Install Doxygen** - Already installed at `C:\Program Files\doxygen\`
1. **Install Graphviz** (for call graphs and architecture diagrams):
   - Download from: <https://graphviz.org/download/>
   - Or via Chocolatey: `choco install graphviz`
   - Or via winget: `winget install graphviz`
   - Add to PATH: `C:\Program Files\Graphviz\bin`
1. Run from the repository root:

```powershell
doxygen Doxyfile
```

1. Open generated documentation:

```text
docs/html/index.html
```

**Note:** Documentation generates even without Graphviz, but you'll miss the interactive call graphs, class diagrams, and system architecture visualization.

### Notes

- `Doxyfile` is configured for this library's public headers and examples.
- **Graphviz (`dot`) is now enabled** for automatic call graphs and system architecture diagrams. Install Graphviz to see function call relationships and data flow visualizations.
- `README.md` is used as the Doxygen main page.
- Doxygen module groups are now defined to organize API pages:
  - `Core Runtime`
  - `Protocol Engine`
  - `Data Access Helpers`
  - `Device Model`
  - `RS485 Control`
  - `Callback Hooks`
- Generated `docs/html` is committed in this repository for direct browsing and release packaging.
- GitHub Pages deployment is automated via `.github/workflows/deploy-docs-pages.yml`.

### Public Documentation Website (GitHub Pages)

- Public URL: `https://nadeemraiwind.github.io/arduino-modbus-rtu-slave/`
- Source: auto-deployed from `docs/html` on each push to `main`

One-time setup (if Pages is not active yet):

1. Open repository **Settings** -> **Pages**
2. Set **Source** to **GitHub Actions**
3. Trigger deployment by pushing to `main` (or run the workflow manually)

### Optional PDF Manual

If you need a printable manual for industrial clients:

1. In `Doxyfile`, set `GENERATE_LATEX = YES`.
2. Run Doxygen again.
3. Build PDF from `docs/latex` (`make` on Linux/macOS or `make.bat` on Windows with a LaTeX toolchain).

## System Architecture

This library follows a clean layered architecture designed for flexibility from basic polling to expert event-driven systems.

### Main Lifecycle - Layered Architecture

The following diagram shows the complete system architecture with four distinct layers:

@dot
digraph G {
  compound=true;
  node [shape=record, fontname=Helvetica, fontsize=10];
  rankdir=TB;

  subgraph cluster_0 {
    label = "Physical Layer";
    color = grey;
    style = filled;
    fillcolor = "#f0f0f0";
    Serial [label="Stream / HardwareSerial\n(RS485/USB)"];
  }

  subgraph cluster_1 {
    label = "Library Core (modbusSlave)";
    color = blue;
    style = filled;
    fillcolor = "#e6f2ff";
    Parser [label="{ State Machine | IDLE \l RECEIVING \l COMPLETE \l }"];
    CRC [label="CRC-16 Validation"];
    FunctionCode [label="Function Code Router\n(FC01-FC06, FC15, FC16)"];
  }

  subgraph cluster_2 {
    label = "Data Layer (modbusRegBank)";
    color = green;
    style = filled;
    fillcolor = "#e6ffe6";
    Bank [label="{ Register Store | Coils (1-2000) \l Discrete Inputs (10001-12000) \l Input Registers (30001-30125) \l Holding Registers (40001-40125) \l }"];
    Atomic [label="Atomic Lock Guard\n(Read-Modify-Write Protection)"];
    TypedHelpers [label="Typed Helpers\n(Float/Long/String)"];
  }

  subgraph cluster_3 {
    label = "User Application";
    color = orange;
    style = filled;
    fillcolor = "#fff5e6";
    Basic [label="Basic/Intermediate\nPolling Logic\n(get/set in loop)"];
    Expert [label="Expert Level\nCallback Hooks\n(onRead/onWrite)"];
  }

  Serial -> Parser [label="Raw Bytes"];
  Parser -> CRC [label="Frame Buffer"];
  CRC -> FunctionCode [label="Valid Frame"];
  FunctionCode -> Bank [label="Direct R/W"];
  FunctionCode -> Expert [style=dashed, color=orange, label="Event Trigger"];
  Expert -> Bank [label="Validated Update"];
  Basic -> Bank [label="Polling Update"];
  Bank -> Atomic [style=dashed, label="Critical Section"];
  Bank -> TypedHelpers [style=dotted, label="Data Conversion"];
  Bank -> FunctionCode [label="Response Data"];
  FunctionCode -> Parser [label="Build Response"];
  Parser -> Serial [label="TX Frame"];
}
@enddot

### Data Flow Diagram (Simplified)

@dot
digraph G {
  rankdir=LR;
  node [shape=box, fontname="Helvetica", fontsize=10];

  Master [label="Modbus Master\n(PLC/SCADA)", fillcolor=lightgrey, style=filled];
  Serial [label="Serial/RS485\nBuffer", shape=ellipse];
  Parser [label="State Machine\n(run)", color=blue, penwidth=2];
  Callback [label="User Callbacks\n(Expert Level)", color=orange, style=dashed];
  Bank [label="Register Bank\n(Data Store)", color=green];
  User [label="Your Application\n(Basic/Intermediate)", fillcolor=lightgreen, style=filled];

  Master -> Serial [label="Request"];
  Serial -> Parser [label="Byte-by-Byte"];
  Parser -> Callback [label="If registered"];
  Parser -> Bank [label="Direct Read/Write"];
  Callback -> User [label="Event Trigger"];
  User -> Bank [label="get/set"];
  Bank -> Parser [label="Data"];
  Parser -> Serial [label="Response"];
  Serial -> Master [label="Frame"];
}
@enddot

### Architecture Patterns by User Level

#### Basic/Intermediate: Polling Flow

In basic usage, your application follows a simple pattern:

1. **Your Application** writes values to the **Register Bank** using `set()`
2. **Your Application** reads values from the **Register Bank** using `get()`
3. The **State Machine** (`slave.run()`) handles all Modbus communication automatically
4. The **Modbus Master** can read/write registers transparently

This "fire and forget" pattern requires no event handling - just update your registers in `loop()` and call `slave.run()`.

**Example:**

```cpp
void loop() {
  // Your application updates data
  int temperature = readSensor();
  regBank.set(30001, temperature * 10);  // Store as 0.1°C units
  
  // Library handles all Modbus communication
  slave.run();  // Non-blocking
}
```

#### Expert: Event-Driven Flow

In expert usage, the library notifies your code when specific Modbus events occur:

1. **Modbus Master** sends a request
2. **State Machine** parses the frame
3. **Callbacks** fire before reads or after writes (if registered)
4. **Your Application** responds to specific register events
5. **Register Bank** provides the storage layer

This pattern enables just-in-time data sampling, command validation, and audit logging.

**Example:**

```cpp
void onTempRead(word address, modbusDevice* dev) {
  // Sample sensor only when master requests it
  int temp = readSensor();
  dev->set(address, temp * 10);
}

void onSetpointWrite(word address, word value, modbusDevice* dev) {
  // Validate and react immediately to writes
  if (value >= 150 && value <= 300) {
    activateHeater(value);
  }
}

void setup() {
  slave.onRead(30001, onTempRead);      // Read callback
  slave.onWrite(40001, onSetpointWrite); // Write callback
}
```

### Interactive Call Graphs

With Graphviz enabled, Doxygen automatically generates:

- **Call graphs** - Show which functions each method calls
- **Caller graphs** - Show which functions call each method
- **Class diagrams** - Show inheritance relationships

These diagrams are interactive: click any box to jump to that function's documentation. This provides a professional datasheet-quality reference similar to Texas Instruments or STMicroelectronics documentation.

### Why This Architecture Works

**Separation of Concerns:**

- Protocol parsing is isolated in `modbusSlave::run()`
- Data storage is isolated in `modbusRegBank`
- Application logic stays in your sketch

**Progressive Complexity:**

- Beginners use simple `get()`/`set()` polling
- Experts add callbacks for specific registers only
- Both patterns coexist without conflicts

**Industrial Grade:**

- Non-blocking state machine prevents loop stalls
- Atomic transaction API prevents data races
- Callback system enables audit trails and validation

## Project State

- Library code: clean and ready for production use
- Examples: progressive industrial set for deployment workflows
  - `examples/01_Basic_MinimalSlave/01_Basic_MinimalSlave.ino` ← **Start here!**
  - `examples/02_Intermediate_MultiRegister/02_Intermediate_MultiRegister.ino`
  - `examples/03_Advanced_TypedData/03_Advanced_TypedData.ino`
  - `examples/04_Expert_Callbacks/04_Expert_Callbacks.ino`
  - `examples/05_RS485_Hardware/05_RS485_Hardware.ino`
  - `examples/06_Validation_Tests/06_Validation_Tests.ino`
  - `examples/07_ASCII_Protocol_Mode/07_ASCII_Protocol_Mode.ino`
  - `examples/08_Atomic_Integrity/08_Atomic_Integrity.ino`
- Documentation: README.md, STATUS.md, ROADMAP.md, ARCHITECTURE.md
- IDE integration: keywords.txt included
- Examples follow a progressive learning path from basic to industrial-grade, and their comments are aligned with the verified USB Serial0 validation workflow.

## Validation Coverage Status

The current validation set is integration-heavy and focused on USB Serial0 workflows.

The user-facing source of truth is now the combination of:

- this README,
- the example headers in `examples/`, and
- `STATUS.md` for explicit gaps and validation coverage.

### Verified by automated Python + Modbus Poll workflows

- Example 01: Basic RTU read/write flow
- Example 02: Multi-register read/write flow
- Example 03: Typed data map behavior (float/long/string register regions)
- Example 04: Callback behavior and diagnostics mirror register checks
- Example 05: RS485 profile sketch tested in Serial0 USB mode
- Example 06: Validation result-map checks
- Example 07: ASCII mode read/write flow

Important distinction:

- The library implements more features than the current development test suite validates.
- “Supported” in source code is not the same as “fully verified” on hardware.
- The table below reflects the validated scope from automated/manual workflows plus the current example set.

### Validated example results summary

| Example | Primary features exercised | Validation result |
| :-- | :-- | :-- |
| 01 | FC01/02/03/04 basic reads, FC05/06 writes, minimal slave lifecycle | PASS |
| 02 | Multi-register process map, dynamic values, control/status regions | PASS |
| 03 | `setFloat`, `setLong`, `setString`, changing typed values | PASS |
| 04 | `onRead`, `onWrite`, `onUnknownFunction`, diagnostics mirror registers | PASS |
| 05 | RS485-oriented register map, serial transport profile, read/write behavior | PASS in Serial0 USB profile; real DE/RE runtime remains deployment-specific |
| 06 | Address validation, duplicate safety, string helpers, `tryGet`, protocol selection | PASS |
| 07 | ASCII framing, `tryGet`, stream baud handler integration, read/write mirror logic | PASS |

### Recently Updated Example Status (Current Cycle)

The following examples were recently updated and compile-verified on Mega2560, but are still pending a fresh end-to-end runtime retest after the latest edits:

- `03_Advanced_TypedData.ino` (updated, compile verified, runtime retest pending)
- `05_RS485_Hardware.ino` (updated, compile verified, runtime retest pending)
- `06_Validation_Tests.ino` (updated, compile verified, runtime retest pending)
- `08_Atomic_Integrity.ino` (new, compile verified, runtime retest pending)

### Feature coverage assessment

| Feature area | Coverage status | Notes |
| :-- | :-- | :-- |
| Basic Modbus RTU communication | High | RTU reads/writes are well covered across Examples 01-07. Current automated evidence is strongest for FC01/02/03/04/05/06. |
| Modbus ASCII mode | Partial | ASCII mode is validated through Example 07 read/write flow, but not across the full RTU function-code matrix. |
| Register management (`add`, `has`, `get`, `set`) | High | Extensively exercised by the example-driven integration tests. |
| Typed data (`setFloat`, `setLong`, `setString`, `tryGet`) | High | Strong integration coverage exists, but explicit `getFloat` / `getLong` assertions are still missing. |
| Endianness switching | Partial | Default behavior is exercised through typed-data workflows, but programmatic switching to Little Endian and Big Endian Swapped is not asserted in automated tests. |
| Dynamic memory model | Partial | The default dynamic allocation path is exercised in normal examples. Stress or failure-path coverage is limited. |
| Static register pool memory model | Missing | `MODBUS_USE_STATIC_REG_POOL` exists in source, but current tests do not build or run a static-pool configuration. |
| Atomic transaction APIs | Missing | `atomicBegin`, `atomicEnd`, `atomicGet`, `atomicSet`, and `isAtomicLocked` are not covered by current practical tests. |
| RS485 DE/RE hardware control | Missing | The API is implemented and documented, but real pin toggling and timing behavior are not yet validated by the automated workflow. |
| Callback system (`onRead`, `onWrite`) | High | Example 04 verifies practical callback behavior and dynamic value updates. |
| Unknown/custom function handling | Partial | Registration is exercised and documented; direct unsupported-function response-path automation is still missing. |
| Stream transport helpers | Partial | `setStreamBaudHandler(...)` appears in Example 07, but current tests still use HardwareSerial rather than a true alternative `Stream` implementation. |
| Diagnostics (FC08) | Partial | Counters are verified indirectly via mirror registers; direct FC08 requests and `clearDiagnosticsCounters()` are not automated. |
| Non-blocking `slave.run()` engine | Partial | Normal integration tests depend on the state machine working, but no stress, throughput, or worst-case latency benchmarks exist yet. |

### Serial routing and debug rules used by the validated workflow

- Default validated profile: `slave.setPort(Serial)` and use the board USB COM port for Modbus.
- When Modbus uses `Serial`, keep Serial Monitor closed and do not use `Serial.print()` for debug output.
- On boards with only one hardware UART, you must choose between USB Modbus traffic and live Serial debug output.
- On boards with extra UARTs (for example MEGA), you can move Modbus to `Serial1`/`Serial2`/`Serial3` and keep `Serial` for debug prints.
- Opening a USB COM port usually resets the Arduino, so masters should wait about 2 to 3 seconds before the first request.
- The example headers now describe both the validated Serial0 path and the alternate multi-UART debug path.

### Important untested or partially tested areas

- Board matrix not fully covered:
  - Multi-UART routing on MEGA (`Serial1/Serial2/Serial3`) in practical bus tests
  - ESP32 hardware UART variants and board-specific serial behavior
  - Other MCU families beyond current USB Serial0 validation path

- Transport and RS485 control paths not fully covered in runtime tests:
  - `setTxEnablePin(...)`
  - `setTxEnableDelays(...)`
  - `setTxEnableDelaysUs(...)`
  - Real DE/RE timing behavior across baud rates and bus lengths

- SoftwareSerial/custom stream paths not fully covered:
  - `setPort(Stream&)` with non-HardwareSerial transports
  - SoftwareSerial timing/performance under load
  - `setStreamBaudHandler(...)` behavior with custom transports

- Memory model coverage is incomplete:
  - Default dynamic register allocation is exercised in normal use
  - Static register pool mode (`MODBUS_USE_STATIC_REG_POOL`) is not validated by the current workflow

- API coverage gaps:
  - Direct unsupported-function injection test for `onUnknownFunction(...)`
  - Direct FC08 request automation (beyond mirror register checks)
  - Atomic APIs in practical scenarios:
    - `atomicBegin()`, `atomicEnd()`, `atomicGet()`, `atomicSet()`, `isAtomicLocked()`
  - Endianness mode switching automation for:
    - `MODBUS_LITTLE_ENDIAN`
    - `MODBUS_BIG_ENDIAN_SWAPPED`
  - Explicit firmware-side assertions for `getFloat(...)` / `getLong(...)`
  - Compile/run coverage for static pool configuration macros:
    - `MODBUS_USE_STATIC_REG_POOL`
    - `MODBUS_MAX_DIG_REGS`
    - `MODBUS_MAX_ANA_REGS`

### Board Matrix Test Plan

Use this checklist to track board/transport coverage systematically.

| Board/Transport | Current Status | Serial Routing Target | Required Checks | Owner | Status |
| :-- | :-- | :-- | :-- | :-- | :-- |
| UNO (HardwareSerial) | Mostly covered | `Serial` | RTU FC01-06/15/16, FC08 mirror, stability | TBD | [ ] |
| MEGA Serial0 profile | Covered in current workflow | `Serial` | RTU + ASCII examples, callback behavior, validation map | Team | [x] |
| MEGA multi-UART profile | Not covered | `Serial1/Serial2/Serial3` for Modbus + `Serial0` for debug/programming | Port routing, throughput, no crosstalk, regression parity with Serial0 profile | TBD | [ ] |
| ESP32 UARTx profile | Not covered | `Serial1/Serial2` (board-specific mapping) | UART pin mapping, timing, FC01-06/15/16, callback/ASCII paths | TBD | [ ] |
| SoftwareSerial/custom Stream | Not covered | `setPort(Stream&)` | `setStreamBaudHandler`, frame reliability, throughput limits, error behavior | TBD | [ ] |

Suggested acceptance gate per row:

- Python automated test: PASS
- Modbus Poll manual test: PASS
- 15-30 minute stability poll: PASS
- Any new issue documented with root cause + fix

If your deployment depends on these areas, add dedicated board/hardware tests before production release.

## Supported Modbus Function Codes

- `FC01 (0x01)` Read Coils
- `FC02 (0x02)` Read Discrete Inputs
- `FC03 (0x03)` Read Holding Registers
- `FC04 (0x04)` Read Input Registers
- `FC05 (0x05)` Write Single Coil
- `FC06 (0x06)` Write Single Register
- `FC15 (0x0F)` Write Multiple Coils
- `FC16 (0x10)` Write Multiple Registers
- `FC08 (0x08)` Diagnostics (selected subfunctions)

## Library API

### `modbusRegBank`

- `add(word address)`
  - Adds a register address to internal storage.
- `set(word address, word value)`
  - Writes value to an existing register.
  - Digital registers (`<20000`) are stored as `0x00` or `0xFF`.
- `word get(word address)`
  - Reads register value.
- `float getFloat(word address)` / `void setFloat(word address, float value)`
  - Reads/writes IEEE-754 float across two adjacent 16-bit registers.
- `uint32_t getLong(word address)` / `void setLong(word address, uint32_t value)`
  - Reads/writes 32-bit unsigned integer across two adjacent 16-bit registers.
- `word setString(word address, const char* str, word maxRegs = 0)`
  - Writes ASCII string as 2 chars per 16-bit register (high byte then low byte).
- `word getString(word address, char* out, word outSize, word regCount)`
  - Reads ASCII string from register blocks using the same packing format.
- `atomicBegin()` / `atomicEnd()`
  - Brackets a critical read-modify-write section to defer `slave.run()` frame processing while active.
- `word atomicGet(word address)` / `void atomicSet(word address, word value)`
  - Tiny interrupt-guarded wrappers for single register read/write operations.
- `bool has(word address)`
  - Returns true if register exists.

Optional memory model compile-time flags (in `modbus.h`):

- `MODBUS_USE_STATIC_REG_POOL` (`0` default, `1` for no-heap mode)
- `MODBUS_MAX_DIG_REGS` (default `64`)
- `MODBUS_MAX_ANA_REGS` (default `64`)

Namespace-safety option (in `modbus.h`):

- Preferred register-type macros: `MODBUS_DO`, `MODBUS_DI`, `MODBUS_AI`, `MODBUS_AO`
- Legacy short aliases (`DO`, `DI`, `AI`, `AO`) remain enabled for compatibility
- Define `MODBUS_DISABLE_LEGACY_SHORT_NAMES` before including `modbus.h` to suppress legacy short aliases in mixed-library projects

### `modbusDevice` (inherits `modbusRegBank`)

- `setId(byte id)`
  - Sets Modbus slave ID.
- `byte getId()`
  - Returns configured slave ID.

### `modbusSlave`

- `setDevice(modbusDevice *device)`
  - Binds the register bank device to slave engine.
- `modbusDevice* getDevice()`
  - Returns currently bound device pointer.
- `setPort(Stream &port)`
  - Selects any Arduino stream-compatible transport.
- `setPort(HardwareSerial &port)`
  - Selects hardware serial port (`Serial`, `Serial1`, etc.) and enables `setBaud()` reconfiguration.
- `setBaud(word baud)`
  - Sets RTU baud rate.
  - With generic `Stream`, configure transport speed externally before `setPort(Stream&)`.
- `word getBaud()`
  - Returns active baud rate.
- `setTxEnablePin(byte pin, bool activeHigh = true)`
  - Optional RS485 DE/RE control.
- `setTxEnableDelays(word preDelayMs, word postDelayMs)`
  - Optional RS485 pre/post TX delays.
- `setTxEnableDelaysUs(word preDelayUs, word postDelayUs)`
  - Optional microsecond-level RS485 pre/post TX delays.
- `onRead(word address, modbusReadCallback cb)`
  - Registers active read callback for an address. Callback executes before value is returned.
- `onWrite(word address, modbusWriteCallback cb)`
  - Registers active write callback for an address. Callback executes after writes (FC05/06/15/16).
  - **Note:** Coil write callbacks (FC05/FC15) receive normalized values: `0x00FF` (ON) or `0x0000` (OFF), regardless of master's raw data format.
- `onUnknownFunction(modbusUnknownFunctionCallback cb)`
  - Registers custom handler for unsupported function codes.
- `configureEndianness(byte mode)` / `getEndianness()`
  - Configures 32-bit helper word order (`MODBUS_BIG_ENDIAN`, `MODBUS_LITTLE_ENDIAN`, `MODBUS_BIG_ENDIAN_SWAPPED`).
- `getBusMessageCount()` / `getBusCommunicationErrorCount()` / `getSlaveMessageCount()`
  - FC08 diagnostics counters exposed through API.
- `clearDiagnosticsCounters()`
  - Resets diagnostics counters to zero.
- `run()`
  - Non-blocking parser tick. Call continuously in `loop()`.

Recommended critical-section pattern for read-modify-write logic:

```cpp
regBank.atomicBegin();
word current = regBank.get(40001);
word next = current + 1;
regBank.set(40001, next);
regBank.atomicEnd();
```

During `atomicBegin()`/`atomicEnd()`, `slave.run()` defers frame processing for that device, helping avoid application-level overwrite races.

## Register Addressing Model

- Coils: `1..2000`
- Discrete Inputs: `10001..12000`
- Input Registers: `30001..30125`
- Holding Registers: `40001..40125`

Master request offsets follow Modbus RTU convention (0-based start address in frame).

### ASCII Data Packing

String helpers use standard register packing: 2 ASCII characters per 16-bit register.

- Register N high byte: first character
- Register N low byte: second character

Example `"HELLO"`:

- `40001 = 0x4845` (`'H' 'E'`)
- `40002 = 0x4C4C` (`'L' 'L'`)
- `40003 = 0x4F00` (`'O' '\0'`)

## Examples Overview

The library includes 9 primary examples in a progressive learning path from basic to expert, plus one development-only single-file debug sketch:

### Level 1: Basic - Minimal Slave ⭐ START HERE

- **File:** `examples/01_Basic_MinimalSlave/01_Basic_MinimalSlave.ino`
- **Hardware:** Arduino UNO, MEGA, or compatible
- **Connection:** USB (Serial)
- **Slave ID:** 1 | **Baud:** 9600
- **Features:**
  - Absolute minimum working code
  - One register of each type (coil, discrete input, input register, holding register)
  - Clear 5-step setup process
  - Comprehensive quick reference guide
- **Serial notes:**
  - Validated on USB `Serial` with Serial Monitor closed
  - Move Modbus to `Serial1`/`Serial2`/`Serial3` on multi-UART boards if you need live debug output on `Serial`
- **Best for:** First-time users learning Modbus basics

### Level 2: Intermediate - Multi-Register Process

- **File:** `examples/02_Intermediate_MultiRegister/02_Intermediate_MultiRegister.ino`
- **Hardware:** Arduino UNO, MEGA, or compatible
- **Connection:** USB or RS485
- **Slave ID:** 1 | **Baud:** 19200
- **Features:**
  - 5 registers per type (20 registers total)
  - Simulated industrial process (temperature, pressure, flow, humidity, voltage)
  - Coil-to-discrete-input mirroring pattern
  - Dynamic value updates and PLC-like command handling
  - Safe register access patterns
- **Serial notes:**
  - Validated on USB `Serial` without debug prints
  - RS485 and alternate UART routing are supported, but should be retested on target hardware
- **Best for:** Building real automation systems

### Level 3: Advanced - Typed Data (Float/Long/String)

- **File:** `examples/03_Advanced_TypedData/03_Advanced_TypedData.ino`
- **Hardware:** Arduino UNO, MEGA, or compatible
- **Connection:** USB or RS485
- **Slave ID:** 1 | **Baud:** 19200
- **Features:**
  - 32-bit float values (IEEE-754) across register pairs
  - 32-bit long values across register pairs
  - ASCII string storage (2 chars per register)
  - Endianness configuration for multi-vendor compatibility
  - Real-world decimal precision handling
- **Serial notes:**
  - Validated on USB `Serial`
  - If you need Serial Monitor diagnostics, move Modbus to another hardware UART first
- **Best for:** SCADA integration with mixed data types

### Level 4: Expert - Callbacks, Diagnostics, Custom Functions

- **File:** `examples/04_Expert_Callbacks/04_Expert_Callbacks.ino`
- **Hardware:** Arduino MEGA recommended (more RAM for logging)
- **Connection:** USB or RS485
- **Slave ID:** 1 | **Baud:** 19200
- **Features:**
  - `onRead()` callbacks for on-demand value generation
  - `onWrite()` callbacks for immediate command reaction
  - `onUnknownFunction()` for custom/vendor function codes
  - FC08 Diagnostics with counter APIs
  - Event logging system
  - Dynamic RAM monitoring
- **Serial notes:**
  - Validated on USB `Serial` with startup delay after COM open
  - On MEGA-class boards you can route Modbus to `Serial1` and keep `Serial` for debug logging
- **Best for:** Mission-critical systems requiring event hooks and diagnostics

### Level 5: RS485 Hardware - Industrial Deployment

- **File:** `examples/05_RS485_Hardware/05_RS485_Hardware.ino`
- **Hardware:** Arduino + RS485 transceiver (MAX485, MAX3485, etc.)
- **Connection:** RS485 twisted pair
- **Slave ID:** 10 (configurable) | **Baud:** 38400
- **Features:**
  - Complete RS485 wiring guide (pinouts, termination, grounding)
  - DE/RE (Driver Enable) pin control
  - Microsecond-precision TX delays
  - Multi-drop bus topology guidelines
  - Comprehensive troubleshooting guide
  - Hardware comparison (MAX485, MAX3485, SN75176, isolated modules)
- **Serial notes:**
  - Development workflow validated the sketch in USB `Serial` profile
  - Real RS485 deployments should switch to the intended UART and enable `setTxEnablePin(...)` plus timing delays on hardware
- **Best for:** Production RS485 installations and multi-slave networks

### Level 6: Validation Tests - Library Safety and API Checks

- **File:** `examples/06_Validation_Tests/06_Validation_Tests.ino`
- **Hardware:** Arduino UNO, MEGA, or compatible
- **Connection:** USB or alternate UART
- **Slave ID:** 1 | **Baud:** 9600
- **Features:**
  - Address-range validation
  - Duplicate register safety checks
  - String helper coverage
  - `tryGet()` sentinel-safe access validation
  - `setProtocol()` / `getProtocol()` result reporting
  - Result-map export over Modbus instead of relying on Serial Monitor
- **Serial notes:**
  - Validated on USB `Serial` as a Modbus-readable status sketch
  - If you move Modbus off `Serial`, you may enable `Serial` debug output for local trace prints
- **Best for:** Regression checking library safety behavior from a Modbus master

### Level 7: Protocol Mode - ASCII + Stream-Oriented Setup

- **File:** `examples/07_ASCII_Protocol_Mode/07_ASCII_Protocol_Mode.ino`
- **Hardware:** Arduino UNO, MEGA, or compatible
- **Connection:** USB or alternate UART
- **Slave ID:** 1 | **Baud:** 9600
- **Features:**
  - ASCII framing via `setProtocol(ASCII)`
  - `tryGet()` usage in active loop logic
  - Optional `setStreamBaudHandler(...)` for generic `Stream` transports
  - Coil mirror behavior from holding-register content
- **Serial notes:**
  - Validated on USB `Serial` using a Modbus ASCII master
  - If you need debug prints, move Modbus to a different hardware UART first
- **Best for:** Legacy gateways, text-framed links, and verifying ASCII mode behavior

### Level 8: Atomic Integrity - Multi-Register Safety

- **File:** `examples/08_Atomic_Integrity/08_Atomic_Integrity.ino`
- **Hardware:** Arduino UNO, MEGA, or compatible
- **Connection:** USB or alternate UART
- **Slave ID:** 1 | **Baud:** 19200
- **Features:**
  - Practical `atomicBegin()` / `atomicEnd()` usage around float/long updates
  - Multi-register consistency under polling load
  - Sequence and cycle-time counters for integrity monitoring
- **Serial notes:**
  - Uses the same Serial0 test profile rules as other validated examples
  - Move Modbus off `Serial` before adding live Serial debug output
- **Best for:** Industrial projects that require race-condition-resistant data exposure

---

**Learning Path Recommendation:**

1. Start with **01_Basic_MinimalSlave** to understand core concepts
2. Move to **02_Intermediate_MultiRegister** for real process simulation
3. Add **03_Advanced_TypedData** when you need floats/longs/strings
4. Use **04_Expert_Callbacks** for event-driven and diagnostic features
5. Use **05_RS485_Hardware** to prepare for real RS485 deployment
6. Run **06_Validation_Tests** when you want a compact regression/status sketch
7. Use **07_ASCII_Protocol_Mode** when your master requires Modbus ASCII framing
8. Use **08_Atomic_Integrity** when multi-register consistency matters under heavy polling

All examples include:

- Detailed inline comments explaining every line
- Register map documentation
- Testing procedures
- Troubleshooting guides
- Best practices and common pitfalls
- Serial routing notes for `Serial` vs `Serial1`/`Serial2`/`Serial3`
- Guidance on when debug output is safe and when it will break Modbus traffic

Manual Modbus Poll guides are available in:

- `extras/tools/MODBUS_POLL_GUIDE_EXAMPLE03.md`
- `extras/tools/MODBUS_POLL_GUIDE_EXAMPLE04.md`

## Safe Register Access

**IMPORTANT:** Always use `has()` before `get()` to avoid ambiguity:

```cpp
// ✅ CORRECT - Safe register access
if (regBank.has(40001)) {
    word value = regBank.get(40001);  // Safe - register exists
    // Use value...
} else {
    // Handle error - register doesn't exist
}

// ⚠️ Direct read without existence check
word value = regBank.get(40001);  // Returns MODBUS_REG_NOT_FOUND (0xFFFF) if missing
```

The `get()` method returns `MODBUS_REG_NOT_FOUND` (`0xFFFF`) if a register doesn't exist. Since `0xFFFF` can also be a valid register value, always check existence first with `has()`.

## Active Callbacks and Diagnostics

```cpp
word readTemperature(word address, modbusDevice* dev) {
  (void)address;
  // Example: sample sensor on demand and return x10 value.
  return 253;  // 25.3 C
}

void writeSetpoint(word address, word value, modbusDevice* dev) {
  (void)address;
  // Example: react immediately to Modbus writes.
  dev->set(40010, value);
}

void setupCallbacks(modbusSlave& slave) {
  slave.onRead(30001, readTemperature);
  slave.onWrite(40001, writeSetpoint);
}

// FC08 diagnostics subfunctions implemented:
// 0x0000 Return Query Data
// 0x000B Bus Message Count
// 0x000C Bus Communication Error Count
// 0x000E Slave Message Count
//
// Optional 32-bit word order compatibility:
// slave.configureEndianness(MODBUS_BIG_ENDIAN);
// slave.configureEndianness(MODBUS_LITTLE_ENDIAN);
// slave.configureEndianness(MODBUS_BIG_ENDIAN_SWAPPED);
```

## Known Limitations

This library is production-ready for most applications, but has some known limitations:

- **Function codes:** FC01-FC06 plus FC15/FC16 are supported
- **Performance:** Default mode uses linked-list storage (O(n) lookup). For long uptime, enable static pools (`MODBUS_USE_STATIC_REG_POOL=1`)
- **Return values:** `get()` returns `MODBUS_REG_NOT_FOUND` (`0xFFFF`) when missing, so use `has()` for robust checks
- **Serial port:** `Stream` is supported. Use `setPort(Stream&)` for SoftwareSerial/bridges; use `setPort(HardwareSerial&)` when baud reconfiguration is needed
- **Timing:** RTU frame detection uses microsecond timing for 3.5-char silence windows

See [STATUS.md](STATUS.md) for complete validation scope and deployment guidance.

## Wiring

### Arduino UNO (Serial0)

- UNO `RX` (pin 0) <= Master TX  
- UNO `TX` (pin 1) => Master RX
- Common GND required
- Baud: `9600`
- Mode: RTU (`8N1`)

**Note:** Cannot use Serial Monitor while connected to Modbus master

### Arduino MEGA (Serial1)

- MEGA `RX1` (pin 19 on some boards / check board silk) <= Master TX
- MEGA `TX1` (pin 18 on some boards / check board silk) => Master RX
- Common GND required
- Baud: `9600`
- Mode: RTU (`8N1`)

Note: Verify pin labels on your exact MEGA board revision; some silkscreen conventions show `TX1=18`, `RX1=19`.

## Minimal Integration Pattern

```cpp
#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

modbusDevice regBank;
modbusSlave slave;

void setup() {
  regBank.setId(1);

  regBank.add(1);      // Coil
  regBank.add(10001);  // Discrete input
  regBank.add(30001);  // Input register
  regBank.add(40001);  // Holding register

  regBank.set(1, 0);
  regBank.set(10001, 0);
  regBank.set(30001, 250);
  regBank.set(40001, 300);

  slave.setDevice(&regBank);
  slave.setPort(Serial1);
  slave.setBaud(9600);
}

void loop() {
  // Update process values
  regBank.set(30001, regBank.get(30001) + 1);

  // Required Modbus handler
  slave.run();
}
```

## Protocol Notes

- CRC16: Modbus polynomial `0xA001`
- Register payload byte order: Big-endian (high byte then low byte)
- Write responses (`FC05`, `FC06`) echo request body on success
- Exceptions are sent with function code ORed by `0x80`

## Production Validation Checklist

Use this checklist before deploying to field hardware:

1. Compile and upload `examples/01_Basic_MinimalSlave/01_Basic_MinimalSlave.ino`.
2. Verify FC01/FC02/FC03/FC04 reads return valid data.
3. Verify FC05/FC06 writes and readback behavior.
4. Verify FC15/FC16 multi-write behavior.
5. If using USB `Serial` for Modbus, keep Serial Monitor closed and allow 2 to 3 seconds after COM open before the first request.
6. If you need live debug logging, move Modbus to a different hardware UART before adding `Serial.print()` calls.
7. If using RS485, validate DE/RE timing and bus stability with `examples/05_RS485_Hardware/05_RS485_Hardware.ino`.
8. Run `examples/06_Validation_Tests/06_Validation_Tests.ino` as a regression gate after transport changes.
9. Confirm slave address, baud, protocol mode, and wiring in your final hardware setup.

## Clean Project Structure

```text
src/
  modbus.h
  modbusDevice.h
  modbusDevice.cpp
  modbusRegBank.h
  modbusRegBank.cpp
  modbusSlave.h
  modbusSlave.cpp
library.properties
keywords.txt
LICENSE
README.md
STATUS.md
ROADMAP.md
ARCHITECTURE.md
Doxyfile
docs/
extras/
examples/
  01_Basic_MinimalSlave/
    01_Basic_MinimalSlave.ino
  02_Intermediate_MultiRegister/
    02_Intermediate_MultiRegister.ino
  03_Advanced_TypedData/
    03_Advanced_TypedData.ino
  04_Expert_Callbacks/
    04_Expert_Callbacks.ino
  05_RS485_Hardware/
    05_RS485_Hardware.ino
  06_Validation_Tests/
    06_Validation_Tests.ino
  07_ASCII_Protocol_Mode/
    07_ASCII_Protocol_Mode.ino
  08_Atomic_Integrity/
    08_Atomic_Integrity.ino
```

## Next Steps

- **Beginners:** Start with `01_Basic_MinimalSlave` to learn core concepts
- **Process control:** Use `02_Intermediate_MultiRegister` for multi-register systems
- **Data types:** Add `03_Advanced_TypedData` for float/long/string support
- **Advanced features:** Implement `04_Expert_Callbacks` for event hooks and diagnostics
- **Hardware deployment:** Move to `05_RS485_Hardware` for industrial RS485 networks
- **Regression checking:** Use `06_Validation_Tests` after transport or API changes
- **ASCII links:** Use `07_ASCII_Protocol_Mode` for text-framed masters and gateways
- **Data integrity:** Use `08_Atomic_Integrity` for multi-register atomic update patterns

## Elite Level Features

This library now includes professional-grade enhancements for industrial deployment:

### ✅ Atomic Transaction Protection

Protects against data races during read-modify-write operations:

- `atomicBegin()` / `atomicEnd()` bracket critical sections
- `atomicGet()` / `atomicSet()` for interrupt-guarded single operations
- `slave.run()` automatically defers frame processing during atomic locks
- **Why it matters:** Prevents master writes from corrupting application logic during complex loop calculations
- Rare feature in Arduino Modbus libraries - designed for mission-critical industrial systems

### ✅ Professional Doxygen Documentation

Complete HTML API reference generated from code:

- Automatic API reference with searchable function documentation
- Organized module groups (Core Runtime, Protocol Engine, Data Access, Device Model, RS485 Control, Callback Hooks)
- Class relationship diagrams
- Professional presentation for open-source portfolio
- Generate locally with `doxygen Doxyfile` → `docs/html/index.html`

## Future Enhancements

Ready to take this library even further? Consider these advanced projects:

### 🏆 Modbus Master Library

Create a companion library for master/client functionality:

- Enable Arduino-to-Arduino Modbus networks
- Poll multiple slaves from a single master
- Bidirectional communication and data aggregation
- Perfect for multi-board distributed systems

See [ROADMAP.md](ROADMAP.md) for complete roadmap.

---

## Author & Maintainer

**Nadeem Abbas**  
📧 Email: [zahid_printers@gmail.com](mailto:zahid_printers@gmail.com)  
📱 Mobile (E.164): +923924509295  
📍 Location: Raiwind, Lahore, Pakistan

### Development Credits

This library was enhanced and documented with assistance from:

- 🤖 **Google Gemini** - AI-assisted code refinement and architecture design
- 🤖 **GitHub Copilot** - Intelligent code completion and documentation generation

### Special Thanks

**Jason Vreeland [CodeRage]**  
Original Beta library foundation  
Released: March 7, 2010 under GNU License

This current library is inspired by and represents a significantly enhanced version of Jason's original work. The core concepts from the Beta library have been modernized and expanded with:

- Industrial-grade validation and error handling
- Atomic transaction protection
- Non-blocking state machine architecture
- Professional UML documentation
- FC15/FC16 multi-write support
- Microsecond-precision RTU timing
- Comprehensive callback system

*We are grateful for Jason's pioneering work that laid the foundation for this industrial-grade implementation.*

---

## Repository & Contributions

### GitHub Repository

🔗 **Main Repository:** [https://github.com/nadeemraiwind/arduino-modbus-rtu-slave](https://github.com/nadeemraiwind/arduino-modbus-rtu-slave)

### How to Contribute

We welcome contributions! Here's how you can help:

#### 🐛 Bug Reports

1. **Check Existing Issues:** Search [GitHub Issues](https://github.com/nadeemraiwind/arduino-modbus-rtu-slave/issues)  first
2. **Create New Issue:** Use the built-in GitHub bug report template
3. **Include Details:**
   - Arduino board model and version
   - Library version (currently 1.1.0)
   - Minimal code to reproduce the issue
   - Expected vs actual behavior
   - Serial output or error messages

**Report bugs:** <https://github.com/nadeemraiwind/arduino-modbus-rtu-slave/issues/new>

Templates available in repository:

- `.github/ISSUE_TEMPLATE/bug_report.md`
- `.github/ISSUE_TEMPLATE/feature_request.md`

#### 💡 Feature Suggestions

1. **Open a Discussion:** Use [GitHub Discussions](https://github.com/nadeemraiwind/arduino-modbus-rtu-slave/discussions) for ideas
2. **Explain Use Case:** Describe the industrial scenario or application
3. **Propose Implementation:** If you have technical details, share them

#### 🔧 Pull Requests

1. **Fork the Repository**
2. **Create Feature Branch:** `git checkout -b feature/your-feature-name`
3. **Follow Code Style:** Match existing formatting and documentation patterns
4. **Test Thoroughly:** Verify on at least one Arduino board
5. **Update Documentation:** Add to README.md and ARCHITECTURE.md if needed
6. **Submit PR:** Reference any related issues

**Contribution Guidelines:**

- Keep changes focused and atomic
- Maintain backward compatibility when possible
- Add Doxygen comments for new public methods
- Update examples if API changes
- Industrial-grade code quality expected

### Community Support

- **Questions:** Open a [GitHub Discussion](https://github.com/nadeemraiwind/arduino-modbus-rtu-slave/discussions)
- **Email Support:** <zahid_printers@gmail.com> (for commercial/industrial inquiries)
- **Documentation:** See [ARCHITECTURE.md](ARCHITECTURE.md) for deep technical details

---

## License

MIT License - See [LICENSE](LICENSE) file for details.

Copyright (c) 2026 Nadeem Abbas and Contributors
