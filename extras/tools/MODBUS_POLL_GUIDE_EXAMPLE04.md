# Modbus Poll Verification Guide - Example 04 (Expert Callbacks)

@page modbus_poll_example04 Modbus Poll Verification Guide - Example 04 (Expert Callbacks)

Status: callback and diagnostics workflow.

## Setup

1. Upload Example 04.
2. Close Serial Monitor.
3. Connect Modbus Poll to board COM port.

Connection:

- Baud: 19200
- 8N1
- Slave ID: 1
- Mode: RTU

## Serial0 USB Rules (Load-Bearing)

- COM open resets Arduino on many boards.
- Wait about 2.5 seconds after connect before first poll.
- Keep Serial Monitor closed while Modbus Poll runs.

## Addressing Rule (Zero-Based)

- 30001 -> address 0 (FC04)
- 40001 -> address 0 (FC03/FC06)
- Coil 00001 -> address 0 (FC01/FC05)

## Tests

1. Read callbacks (FC04 start 0 qty 3)

- 30001 temp moves over reads
- 30002 uptime increases
- 30003 free RAM is valid on AVR

1. Write callback + clamp

- Write 40001=1500, read back 1500
- Write 40001=5000, read back clamped 3000

1. Alarm reset callback

- Write 40002=1
- Read back 40002=0 (auto-reset)

1. Coil callback

- Write coil 00001 ON/OFF and verify state

1. Diagnostics mirrors

- Read 40010-40012
- Message counters should increase
- Error counter should stay low/zero on healthy link

## Pass Criteria

- No recurring timeout/CRC errors
- Reads and writes succeed
- Clamp/reset behavior is correct
- Diagnostics counters behave logically
