# Example 06 - Validation Tests Register Guide

This example is a self-test/regression sketch.
It reports validation status through holding registers so users can verify results without Python scripts.

## Connection

- Slave ID: 1
- Baud: 9600
- Framing: RTU
- Use zero-based addressing in master tools.

## Serial0 USB Rules

- Opening COM usually resets Arduino.
- Wait about 2.5 seconds after connect before first read.
- Keep Serial Monitor closed while Modbus traffic is active on Serial.

## Result Register Map

- 40001: Suite complete flag
  - 0 = running / not started
  - 1 = complete
- 40002: Passed test count
- 40003: Failed test count
- 40004: Active or last test number
- 40005: Last error code
  - 0 = no error
- 40006: Active protocol mode
  - RTU constant value as reported by library
- 40011-40017: Per-test status
  - 0 = not run
  - 1 = pass
  - 2 = fail

## Per-Test Meaning

- 40011: Test 1 - Valid address ranges
- 40012: Test 2 - Invalid address rejection
- 40013: Test 3 - Memory pool safety checks
- 40014: Test 4 - String helper operations
- 40015: Test 5 - Timing rollover arithmetic
- 40016: Test 6 - tryGet sentinel-safe behavior
- 40017: Test 7 - Protocol selection API

## Expected Healthy Snapshot

- 40001 = 1
- 40002 = 7
- 40003 = 0
- 40005 = 0
- 40011..40017 = all 1
