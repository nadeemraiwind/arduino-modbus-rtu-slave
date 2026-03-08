# Modbus RTU Arduino Library

Standard Modbus RTU slave library for Arduino with support for core Modbus function codes and flexible register-bank usage.

## Version 1.1.0

**Recent Improvements:**
- ✅ Fixed CRC table linker errors (moved from .h to .cpp)
- ✅ Fixed broken example code to use correct API
- ✅ Added comprehensive inline documentation
- ✅ Created KNOWN_ISSUES.md with limitations and best practices
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

See [KNOWN_ISSUES.md](KNOWN_ISSUES.md) for limitations and best practices.

## Doxygen Documentation

This repository now includes a ready-to-run `Doxyfile`.

### Generate HTML Docs

1. **Install Doxygen** - Already installed at `C:\Program Files\doxygen\`
2. **Install Graphviz** (for call graphs and architecture diagrams):
   - Download from: https://graphviz.org/download/
   - Or via Chocolatey: `choco install graphviz`
   - Or via winget: `winget install graphviz`
   - Add to PATH: `C:\Program Files\Graphviz\bin`
3. Run from the repository root:

```powershell
doxygen Doxyfile
```

4. Open generated documentation:

```text
docs/html/index.html
```

**Note:** Documentation generates even without Graphviz, but you'll miss the interactive call graphs, class diagrams, and system architecture visualization.

### Notes

- `Doxyfile` is configured for this library's public headers and examples.
- **Graphviz (`dot`) is now enabled** for automatic call graphs and system architecture diagrams. Install Graphviz to see function call relationships and data flow visualizations.
- `README.md` is used as the Doxygen main page.
- Doxygen module groups are now defined to organize API pages:
  - `Protocol Engine`
  - `Data Access Helpers`
  - `Device Model`
  - `RS485 Control`
- Generated `docs/html` is intentionally ignored by Git; regenerate locally as needed.
- For hosted docs, use GitHub Pages from a dedicated docs branch/workflow output.

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
- Examples: 5 progressive examples (Basic → Expert)
  - `examples/01_Basic_MinimalSlave/01_Basic_MinimalSlave.ino` ← **Start here!**
  - `examples/02_Intermediate_MultiRegister/02_Intermediate_MultiRegister.ino`
  - `examples/03_Advanced_TypedData/03_Advanced_TypedData.ino`
  - `examples/04_Expert_Callbacks/04_Expert_Callbacks.ino`
  - `examples/05_RS485_Hardware/05_RS485_Hardware.ino`
- Documentation: README.md, KNOWN_ISSUES.md
- IDE integration: keywords.txt included
- Examples follow progressive learning path from basic to industrial-grade

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

The library includes 5 examples in a progressive learning path from basic to expert:

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
- **Best for:** Production RS485 installations and multi-slave networks

---

**Learning Path Recommendation:**
1. Start with **01_Basic_MinimalSlave** to understand core concepts
2. Move to **02_Intermediate_MultiRegister** for real process simulation
3. Add **03_Advanced_TypedData** when you need floats/longs/strings
4. Use **04_Expert_Callbacks** for event-driven and diagnostic features
5. Deploy with **05_RS485_Hardware** for industrial installations

All examples include:
- Detailed inline comments explaining every line
- Register map documentation
- Testing procedures
- Troubleshooting guides
- Best practices and common pitfalls

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

See [KNOWN_ISSUES.md](KNOWN_ISSUES.md) for complete details and workarounds.

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
5. If using RS485, validate DE/RE timing and bus stability with `examples/05_RS485_Hardware/05_RS485_Hardware.ino`.
6. Confirm slave address, baud, and wiring in your final hardware setup.

## Clean Project Structure

```text
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
KNOWN_ISSUES.md
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
```

## Next Steps

- **Beginners:** Start with `01_Basic_MinimalSlave` to learn core concepts
- **Process control:** Use `02_Intermediate_MultiRegister` for multi-register systems
- **Data types:** Add `03_Advanced_TypedData` for float/long/string support
- **Advanced features:** Implement `04_Expert_Callbacks` for event hooks and diagnostics
- **Production deployment:** Deploy with `05_RS485_Hardware` for industrial RS485 networks

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
- Organized module groups (Protocol Engine, Data Access, Device Model, RS485 Control)
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

See [KNOWN_ISSUES.md](KNOWN_ISSUES.md#future-enhancements-suggestions) for complete roadmap.
