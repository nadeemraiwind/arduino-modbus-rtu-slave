# Known Issues and Limitations

This document tracks known limitations, practical workarounds, and recent fixes.

## Current Status

The following major items have already been implemented in code:

- Sentinel-safe register reads via `tryGet(word, word*)`
- Sequential lookup cache for linked-list register searches
- Early frame completion for fixed-length RTU requests (FC01-FC06)
- Stream baud callback for non-`HardwareSerial` transports
- Optional Modbus ASCII mode (`setProtocol(ASCII)`)

## Active Limitations

### 1. Linked-List Storage Cost at Scale

- Register storage still uses linked lists by default.
- Lookup remains O(n) in the worst case (cache helps common sequential access).
- Large maps can increase latency and RAM overhead.

Workarounds:

- Keep register maps compact and grouped.
- Use static register pool mode for deterministic memory behavior:
  - `MODBUS_USE_STATIC_REG_POOL=1`
  - `MODBUS_MAX_DIG_REGS=<n>`
  - `MODBUS_MAX_ANA_REGS=<n>`

### 2. RTU Timing Still Requires a Fast `loop()`

- Parser behavior is non-blocking and byte-wise.
- RTU frame boundary detection still depends on silence-window timing.
- Long blocking calls in `loop()` can reduce request throughput.

Workarounds:

- Call `slave.run()` frequently.
- Replace `delay()` with non-blocking timing patterns (`millis()` based).

### 3. Generic `Stream` Does Not Have Standardized Baud Control

- `setBaud()` can directly reconfigure only `HardwareSerial`.
- Generic `Stream` transport speed must be configured by user code.

Workaround:

- Register `setStreamBaudHandler(...)` when using custom transports and reconfigure the transport in the callback.

## Behavior Notes

### FC01-FC06 Request Length Enforcement

- Fixed-length request validation for FC01-FC06 remains strict by design.
- This is protocol-correct behavior (8-byte RTU request frame for these function codes).

### `get()` vs `tryGet()`

- `get()` preserves compatibility and returns `MODBUS_REG_NOT_FOUND` for missing registers.
- `tryGet()` should be preferred for unambiguous reads.

Example:

```cpp
word value = 0;
if (regBank.tryGet(40001, &value)) {
  // Safe: register exists and value is valid (including 0xFFFF)
}
```

## Supported Function Codes

| Code | Hex  | Function                        | Status    |
| :--- | :--- | :------------------------------ | :-------- |
| FC01 | 0x01 | Read Coils                      | Supported |
| FC02 | 0x02 | Read Discrete Inputs            | Supported |
| FC03 | 0x03 | Read Holding Registers          | Supported |
| FC04 | 0x04 | Read Input Registers            | Supported |
| FC05 | 0x05 | Write Single Coil               | Supported |
| FC06 | 0x06 | Write Single Holding Register   | Supported |
| FC08 | 0x08 | Diagnostics (subset)            | Supported |
| FC15 | 0x0F | Write Multiple Coils            | Supported |
| FC16 | 0x10 | Write Multiple Holding Registers| Supported |

## Best Practices

1. Use `tryGet()` for read paths where missing registers are possible.
2. Keep `loop()` non-blocking and call `slave.run()` as often as possible.
3. For RS485, configure DE/RE timing and test at target baud rate.
4. Validate all register addresses against Modicon ranges.

## Roadmap Ideas

1. Optional array-backed storage mode with binary search.
2. Optional compile-time profiles for low-RAM boards.
3. Additional diagnostics counters and telemetry hooks.

## Report Issues

- Issues: [GitHub Issues](https://github.com/nadeemraiwind/arduino-modbus-rtu-slave/issues/new)
- Discussions: [GitHub Discussions](https://github.com/nadeemraiwind/arduino-modbus-rtu-slave/discussions)
- Email: `zahid_printers@gmail.com`
