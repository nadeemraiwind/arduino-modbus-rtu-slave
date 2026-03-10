# Modbus Poll Testing Guide - Example 03 (Typed Data)

@page modbus_poll_example03 Modbus Poll Testing Guide - Example 03 (Typed Data)

## Connection Settings

- Port: COM port used by your Arduino
- Baud: 19200
- Data: 8 bits, no parity, 1 stop bit
- Mode: RTU
- Slave ID: 1
- Timeout: 2000 ms

## Serial0 USB Rules (Load-Bearing)

- Opening the serial COM port resets many Arduino boards.
- Wait about 2.5 seconds after connect before first request.
- Keep Arduino Serial Monitor closed while Modbus Poll is connected.

## Addressing Rule (Zero-Based)

- Modicon 40001 = start address 0
- Modicon 40011 = start address 10
- Modicon 40021 = start address 20

## Window 1: Float Block

Settings:

- Function: FC03 Read Holding Registers
- Start: 0
- Quantity: 8
- Display: 32-bit Float
- Byte order: Big Endian (default)

Map:

- 40001-40002 Temperature
- 40003-40004 Pressure
- 40005-40006 Flow Rate
- 40007-40008 Humidity

## Window 2: Long Block

Settings:

- Function: FC03
- Start: 10
- Quantity: 6
- Display: 32-bit Signed Integer

Map:

- 40011-40012 Runtime
- 40013-40014 Pulse Counter
- 40015-40016 Error Code

## Window 3: String Block

Settings:

- Function: FC03
- Start: 20
- Quantity: 15
- Display: ASCII/String (or Hex and decode manually)

Map:

- 40021-40030 Device Name
- 40031-40035 Status Message

## Troubleshooting

- Garbage float/long values: switch byte/word order.
- Timeouts: verify baud 19200, close other COM users, increase timeout.
- String noise: verify start address 20 and proper ASCII interpretation.

## Quick Checklist

- Connect at 19200, slave ID 1.
- Read 0/8 as float values.
- Read 10/6 as long values.
- Read 20/15 as strings.
- Verify dynamic fields change over time.
