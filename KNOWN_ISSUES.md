# Known Issues and Limitations

This document describes the current state of the library, recent fixes, and known limitations.

## Recent Fixes (v1.1.0)

### ✅ **ADDED: Optional Static Register Pool Mode**

- **Added**: Compile-time static pool mode to avoid heap allocation in register creation.
- **Config**: `MODBUS_USE_STATIC_REG_POOL`, `MODBUS_MAX_DIG_REGS`, `MODBUS_MAX_ANA_REGS`.
- **Impact**: Better long-term stability for always-on industrial deployments.

### ✅ **FIXED: Broken Example Code**
- **Issue**: The old simple example used non-existent methods (`addCoil()`, `addIsts()`, `slave.begin()`).
- **Fix**: Updated example to use correct API (`add()`, `setDevice()`, `setPort()`)
- **Impact**: Users can now compile the simple example without errors

### ✅ **FIXED: CRC Table Linker Errors**
- **Issue**: CRC lookup tables were defined in `modbusSlave.h`, causing "multiple definition" errors when including the header in multiple files
- **Fix**: Moved CRC table definitions to `modbusSlave.cpp` and declared them as `extern const` in the header
- **Impact**: Library can now be used in multi-file projects without linker errors

### ✅ **IMPROVED: Example Documentation**
- **Issue**: Limited inline documentation about how to use the library
- **Fix**: Added comprehensive comments to `examples/01_Basic_MinimalSlave/01_Basic_MinimalSlave.ino` explaining:
  - Modicon address ranges (1-9999 for coils, 10001-19999 for discrete inputs, etc.)
  - How to add more registers
  - RS485 DE/RE control pin configuration
  - Safe register access using `has()` method
  - Supported Modbus function codes
- **Impact**: New users can understand the API more quickly

### ✅ **FIXED: 8-Byte Read Assumption in run()**

- **Issue**: `run()` previously read a hardcoded 8 bytes from UART.
- **Fix**: `run()` now detects a complete frame first (`checkSerial()`), then reads the detected frame length (`serialRx()`).
- **Impact**: Frame intake is no longer hardcoded to 8 bytes, making request parsing more robust.

### ✅ **IMPROVED: Microsecond RTU Frame Timing**

- **Issue**: Millisecond-based frame timing was too coarse at high baud rates.
- **Fix**: Frame silence detection now uses `micros()` with a 3.5-character RTU window.
- **Impact**: Better behavior at higher baud rates and on timing-sensitive masters.

### ✅ **ADDED: FC15/FC16 Multi-Write Support**

- **Added**: FC15 (`0x0F`, Write Multiple Coils) and FC16 (`0x10`, Write Multiple Holding Registers).
- **Validation**: Request byte-count and payload-length checks were added for both functions.
- **Response**: Standard Modbus write-multiple response (echo start address + quantity).

### ✅ **ADDED: Stream Transport Support**

- **Added**: `setPort(Stream&)` in addition to `setPort(HardwareSerial&)`.
- **Impact**: Library can now work with stream-based transports (SoftwareSerial, bridges, etc.).
- **Note**: `setBaud()` actively reconfigures UART only when using `HardwareSerial`.

### ✅ **IMPROVED: get() Not-Found Signaling**

- **Issue**: `get()` previously returned `0` for missing registers.
- **Fix**: `get()` now returns `MODBUS_REG_NOT_FOUND` (`0xFFFF`) when a register is not present.
- **Impact**: Missing-register reads are explicit; use `has()` for fully robust checks.

## Current Limitations

### ⚠️ **Fixed-Length Request Validation for FC01-FC06**

- **Issue**: FC01-FC06 validation still enforces 8-byte request size, which is correct for these function codes.
- **Impact**: FC15/FC16 have their own variable-length validators, while FC01-FC06 remain strictly 8-byte as required.

### ⚠️ **Sentinel Collision Risk in get()**

- **Issue**: `modbusRegBank::get()` returns `MODBUS_REG_NOT_FOUND` (`0xFFFF`) for missing registers, but `0xFFFF` may also be valid register data.
- **Workaround**: **Always use `has()` before `get()`** to verify register existence:
  ```cpp
  if (regBank.has(40001)) {
    word value = regBank.get(40001);  // Safe - register exists
    // Use value...
  }
  ```
- **Impact**: Minor - requires defensive programming
- **Future**: Could add `bool get(word addr, word* outValue)` style API.

### ⚠️ **Linked List Lookup Cost (Default Mode)**  
- **Issue**: Default register storage uses linked list (O(n) lookup time)
- **Impact**: 
  - Slow for large register counts (100+ registers)
  - Higher RAM overhead (~6 bytes per register + malloc overhead)
  - Potential heap fragmentation on long-running systems
- **Benefit**: Allows sparse addressing (e.g., address 1 and 40001 without gaps)
- **Workaround**: Enable static pool mode (`MODBUS_USE_STATIC_REG_POOL=1`) and keep register count sized to configured pools.
- **Future**: Could add sorted-array/binary-search mode.

### ⚠️ **Frame Completion Depends on RTU Silence Window**
- **Issue**: The parser is now non-blocking and byte-wise, but frame completion still relies on RTU silent interval detection.
- **Impact**: If user code blocks for long periods in `loop()`, timing jitter can still affect request cadence.
- **Workaround**: Keep polling loop tight and avoid long blocking code in `loop()`.

### ⚠️ **Baud Reconfiguration Depends on HardwareSerial**
- **Issue**: `setBaud()` can only call `begin()` when the selected port is `HardwareSerial`.
- **Impact**: With generic `Stream` transports, baud/radio settings must be configured externally by the user.
- **Workaround**: Configure non-HardwareSerial transport speed in user code before calling `setPort(Stream&)`.

### ℹ️ **No Modbus ASCII Support**
- **Issue**: `modbus.h` defines `ASCII` constant but there is no implementation
- **Impact**: None - constant is unused
- **Recommendation**: Constant should be removed to avoid confusion, or ASCII mode should be implemented

## Best Practices

### Register Management
1. **Always check register existence:**
   ```cpp
   if (regBank.has(40001)) {
       word value = regBank.get(40001);
       // Safe to use value
   }
   ```

2. **Use sparse addressing wisely:**
   - Linked list allows gaps (coil 1, coil 100) without wasting memory
   - But too many registers slows down lookups
   - Recommended: <50 total registers for good performance

3. **Follow Modicon address conventions:**
   - Coils: 1-9999
   - Discrete Inputs: 10001-19999
   - Input Registers: 30001-39999
   - Holding Registers: 40001-49999

### Serial Communication
1. **Use recommended baud rates:**
   - 9600 (most reliable)
   - 19200 (good balance)
   - 38400 (advanced use only)

2. **For RS485:**
   ```cpp
   slave.setTxEnablePin(2, true);     // Pin 2, active HIGH
   slave.setTxEnableDelays(0, 1);     // 0ms pre, 1ms post
   ```

3. **Test with single-register operations first** before attempting complex scenarios

### Memory Management
1. **Watch RAM usage:** Each register uses ~6 bytes + malloc overhead
2. **On Arduino Uno (2KB RAM):** Keep total registers under 100
3. **On Arduino MEGA (8KB RAM):** Can handle more but still monitor usage

## Supported Function Codes

| Code | Hex  | Function                      | Status      |
|------|------|-------------------------------|-------------|
| FC01 | 0x01 | Read Coils                    | ✅ Supported |
| FC02 | 0x02 | Read Discrete Inputs          | ✅ Supported |
| FC03 | 0x03 | Read Holding Registers        | ✅ Supported |
| FC04 | 0x04 | Read Input Registers          | ✅ Supported |
| FC05 | 0x05 | Write Single Coil             | ✅ Supported |
| FC06 | 0x06 | Write Single Holding Register | ✅ Supported |
| FC15 | 0x0F | Write Multiple Coils          | ✅ Supported |
| FC16 | 0x10 | Write Multiple Registers      | ✅ Supported |

## Testing Recommendations

1. **Start with the simple example** to verify basic operation
2. **Test each function code individually** before combining
3. **Use a Modbus master tool** (like Modbus Poll or pymodbus) for testing
4. **Monitor Serial output** during debug phase
5. **Test RS485 direction control** thoroughly if using RS485 hardware

## Future Enhancements (Suggestions)

### Elite Level (Major Features)

1. 🏆 **Doxygen Documentation**
   - Generate professional HTML documentation from code comments
   - Automated API reference with class diagrams
   - Searchable function/method documentation
   - Prerequisites: Add Doxygen-style comments to headers
   - Tools: Doxygen, GraphViz for class diagrams

2. 🏆 **Modbus Master Library**
   - Companion library for master/client functionality
   - Enable Arduino-to-Arduino Modbus networks
   - Support same function codes as slave library
   - Use cases: Multi-board systems, distributed control, data aggregation

### Performance & API Improvements

3. ⭐ Add sorted-array/binary-search register storage option
4. Add `bool get(word addr, word* outValue)` API to eliminate sentinel ambiguity
5. Add configurable exception response policy for unknown function codes
6. Remove or implement ASCII mode constant

## Version History

### v1.1.0 (Current)
- Added non-blocking byte-wise parser state machine
- Added unknown/custom function hook (`onUnknownFunction`)
- Added endianness configuration for 32-bit helpers
- Added FC08 diagnostics subset + counters API
- Added float/long/string register helper functions
- Added FC15/FC16 support
- Added `Stream` transport support (`setPort(Stream&)`)
- Added microsecond RTU frame timing (`micros()` based)
- Added optional static register pool mode (no-heap)
- Added Arduino IDE `keywords.txt`
- Added MIT `LICENSE`
- Added and refreshed example sketches

### v0.1.x (Previous)
- Initial implementation
- FC01-FC06 support
- Basic register bank with linked list
- RS485 DE/RE control support
