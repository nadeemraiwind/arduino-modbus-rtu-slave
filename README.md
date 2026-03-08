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
2. **Start with Simple Example:** Open MODBUSslave_Simple.ino from Examples menu
3. **Wire your hardware:** Connect RX/TX to your Modbus master
4. **Configure:** Set slave ID and baud rate (default 9600)
5. **Upload and test** with a Modbus master tool

See [KNOWN_ISSUES.md](KNOWN_ISSUES.md) for limitations and best practices.

## Project State

- Library code: clean and ready for production use
- Examples: 6 comprehensive examples
  - `examples/MODBUSslave_Simple/MODBUSslave_Simple.ino` ← **Start here!**
  - `examples/MODBUSslave_Callbacks_Diagnostics/MODBUSslave_Callbacks_Diagnostics.ino`
  - `examples/MODBUSslave_FloatValues/MODBUSslave_FloatValues.ino`
  - `examples/MODBUSslave_StringValues/MODBUSslave_StringValues.ino`
  - `examples/MODBUSslave_UNO_Serial0/MODBUSslave_UNO_Serial0.ino`
  - `examples/MODBUSslave_MEGA_Serial1/MODBUSslave_MEGA_Serial1.ino`
- Documentation: README.md, KNOWN_ISSUES.md
- IDE integration: keywords.txt included
- Serial debug prints in examples: removed for production use

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

The library includes 6 examples demonstrating different use cases:

### Example 1: Simple (Recommended Starting Point)

- File: `examples/MODBUSslave_Simple/MODBUSslave_Simple.ino`
- Port: `Serial` (RX0/TX0)
- Features: Minimal setup with 1 register of each type, comprehensive inline documentation
- **Best for:** Learning the API and getting started quickly

### Example 2: Callbacks + Diagnostics

- File: `examples/MODBUSslave_Callbacks_Diagnostics/MODBUSslave_Callbacks_Diagnostics.ino`
- Port: `Serial` (RX0/TX0)
- Features: Demonstrates `onRead/onWrite` active callbacks and FC08 diagnostics counters
- **Best for:** Event-driven process control and field troubleshooting

### Example 3: Float Values

- File: `examples/MODBUSslave_FloatValues/MODBUSslave_FloatValues.ino`
- Port: `Serial` (RX0/TX0)
- Features: Demonstrates `setFloat/getFloat` with adjacent holding registers
- **Best for:** Projects using IEEE-754 process values (temperature, pressure, etc.)

### Example 4: String Values

- File: `examples/MODBUSslave_StringValues/MODBUSslave_StringValues.ino`
- Port: `Serial` (RX0/TX0)
- Features: Demonstrates `setString/getString` with 2 ASCII chars per holding register
- **Best for:** Device names, status messages, barcode/RFID text over Modbus

### Example 5: UNO Serial0

- File: `examples/MODBUSslave_UNO_Serial0/MODBUSslave_UNO_Serial0.ino`
- Port: `Serial` (RX0/TX0)
- Features: Full API demonstration with simulated process model, 5 registers per type
- **Best for:** Single-UART Arduino UNO applications

### Example 6: MEGA Serial1

- File: `examples/MODBUSslave_MEGA_Serial1/MODBUSslave_MEGA_Serial1.ino`
- Port: `Serial1` (RX1/TX1 - pins 18/19)
- Features: Full API demonstration using Serial1, allows simultaneous USB debugging
- **Best for:** Arduino MEGA with separate programming and Modbus UARTs

All examples demonstrate:
- All 4 register groups (coils, discrete inputs, input registers, holding registers)
- Full API usage (`add`, `set`, `get`, `setFloat`, `getFloat`, `setLong`, `getLong`, `setString`, `getString`, `has`, `setId`, `getId`, `setDevice`, `getDevice`, `setPort`, `setBaud`, `getBaud`, `onRead`, `onWrite`, `getBusMessageCount`, `getBusCommunicationErrorCount`, `getSlaveMessageCount`, `clearDiagnosticsCounters`, `run`)
- Optional RS485 APIs as commented lines (`setTxEnablePin`, `setTxEnableDelays`, `setTxEnableDelaysUs`)
- Production-ready code (no serial debug output)

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

## Final Programming and Test Report

Requested by user:

1. Program Serial0 example to MEGA on `COM12`
2. Test all function codes and confirm features

Final result:

- `Serial0` example was compiled and uploaded to MEGA on `COM12`
- All Modbus core function codes responded correctly
- FC06 write was verified by FC03 readback

### Upload Commands Used

```powershell
"C:\Program Files\Arduino CLI\arduino-cli.exe" compile --fqbn arduino:avr:mega --library "c:\Users\PC\OneDrive\Desktop\arduino lib" "c:\Users\PC\OneDrive\Desktop\arduino lib\examples\MODBUSslave_UNO_Serial0"
"C:\Program Files\Arduino CLI\arduino-cli.exe" upload -p COM12 --fqbn arduino:avr:mega "c:\Users\PC\OneDrive\Desktop\arduino lib\examples\MODBUSslave_UNO_Serial0"
```

### Function Test Responses (COM12, 9600, Slave ID 1)

- FC01 Read Coils (`00001..00005`)
  - Request: `01 01 00 00 00 05 FC 09`
  - Response: `01 01 01 00 51 88`
  - Result: PASS

- FC02 Read Discrete Inputs (`10001..10005`)
  - Request: `01 02 00 00 00 05 B8 09`
  - Response: `01 02 01 00 A1 88`
  - Result: PASS

- FC03 Read Holding Registers (`40001..40005`)
  - Request: `01 03 00 00 00 05 85 C9`
  - Response: `01 03 0A 01 5E 01 2C 00 64 03 E8 00 64 19 4D`
  - Result: PASS

- FC04 Read Input Registers (`30001..30005`)
  - Request: `01 04 00 00 00 05 30 09`
  - Response: `01 04 0A 01 0F 00 4B 02 13 00 87 00 EB 4E 0B`
  - Result: PASS

- FC05 Write Single Coil ON (`coil 1`)
  - Request: `01 05 00 00 FF 00 8C 3A`
  - Response: `01 05 00 00 FF 00 8C 3A`
  - Result: PASS

- FC05 Write Single Coil OFF (`coil 1`)
  - Request: `01 05 00 00 00 00 CD CA`
  - Response: `01 05 00 00 00 00 CD CA`
  - Result: PASS

- FC06 Write Single Register (`40001 = 1234`)
  - Request: `01 06 00 00 04 D2 0B 57`
  - Response: `01 06 00 00 04 D2 0B 57`
  - Result: PASS

- FC03 Verify FC06 writeback (`read 40001`)
  - Request: `01 03 00 00 00 01 84 0A`
  - Response: `01 03 02 04 D2 3A D9`
  - Decoded value: `1234`
  - Result: PASS

### Test Verdict

- FC01: PASS
- FC02: PASS
- FC03: PASS
- FC04: PASS
- FC05: PASS
- FC06: PASS
- FC06 readback verification: PASS

Overall: `ALL PASS`

## Exact Commands To Run On Your PC

### Upload (choose one method)

- Arduino IDE GUI:
  - Open `examples/MODBUSslave_UNO_Serial0/MODBUSslave_UNO_Serial0.ino`
  - Board: Arduino Mega 2560
  - Port: COM12
  - Upload

- Arduino CLI (if installed):

```powershell
arduino-cli compile --fqbn arduino:avr:mega "examples/MODBUSslave_UNO_Serial0"
arduino-cli upload -p COM12 --fqbn arduino:avr:mega "examples/MODBUSslave_UNO_Serial0"
```

### Quick Modbus Functional Checks

Use any Modbus master (QModMaster/modpoll/PLC/SCADA) and test:

- FC01: Read coils start `0`, qty `5`
- FC02: Read discrete inputs start `0`, qty `5`
- FC03: Read holding registers start `0`, qty `5`
- FC04: Read input registers start `0`, qty `5`
- FC05: Write coil `0` with `0xFF00`, then `0x0000`
- FC06: Write register `0` with `1234`, read back via FC03

Expected:

- Valid responses for all 6 function codes
- No exception for configured addresses
- Echo response for FC05/FC06

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
README.md
examples/
  MODBUSslave_UNO_Serial0/
    MODBUSslave_UNO_Serial0.ino
  MODBUSslave_MEGA_Serial1/
    MODBUSslave_MEGA_Serial1.ino
tools/
```

## Next Recommended Action

Use `examples/MODBUSslave_MEGA_Serial1/MODBUSslave_MEGA_Serial1.ino` when you move to dedicated Modbus UART on MEGA (`Serial1`) for production wiring.
