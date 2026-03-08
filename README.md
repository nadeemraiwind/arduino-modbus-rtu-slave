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

1. Install Doxygen (and Graphviz for diagrams).
2. Run from the repository root:

```powershell
doxygen Doxyfile
```

3. Open generated documentation:

```text
docs/html/index.html
```

### Notes

- `Doxyfile` is configured for this library's public headers and examples.
- Graphviz (`dot`) is optional and currently disabled by default in `Doxyfile` for clean cross-machine generation.
- `README.md` is used as the Doxygen main page.

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
- `bool has(word address)`
  - Returns true if register exists.

Optional memory model compile-time flags (in `modbus.h`):
- `MODBUS_USE_STATIC_REG_POOL` (`0` default, `1` for no-heap mode)
- `MODBUS_MAX_DIG_REGS` (default `64`)
- `MODBUS_MAX_ANA_REGS` (default `64`)

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

## Elite Level Enhancements

Ready to take this library to the next level? Consider these advanced projects:

### 🏆 Professional Documentation with Doxygen
Generate a complete HTML documentation website from the code:
- Install Doxygen and add structured comments to headers
- Automatic API reference with class diagrams
- Searchable function documentation
- Professional presentation for open-source portfolio

### 🏆 Modbus Master Library
Create a companion library for master/client functionality:
- Enable Arduino-to-Arduino Modbus networks
- Poll multiple slaves from a single master
- Bidirectional communication and data aggregation
- Perfect for multi-board distributed systems

See [KNOWN_ISSUES.md](KNOWN_ISSUES.md#future-enhancements-suggestions) for complete roadmap.
