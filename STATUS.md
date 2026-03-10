# Tested vs Untested Status {#status}

@page status Tested vs Untested Status

This page captures practical validation scope for professional deployments.

## Validation Profile Used

- Serial0 USB profile
- RTU and ASCII example-driven integration checks
- External master tools (Modbus Poll / pymodbus workflows)

## Tested Features

- FC01, FC02, FC03, FC04, FC05, FC06 integration behavior
- FC15, FC16 implemented and compile validated
- RTU mode across primary examples
- ASCII mode via protocol example workflow
- Register bank basics: add, has, get, set, tryGet
- Callback registration and practical onRead/onWrite behavior
- Diagnostics mirrors via holding registers

## Coverage Audit (Python + Manual)

- Automated workflow coverage: primary FC01, FC02, FC03, FC04, FC05, FC06 request/response paths.
- Manual Modbus Poll coverage: typed-data windows, callback behavior, diagnostics mirrors.
- Compile-verified examples with targeted runtime checks: 03, 05, 06, 07, 08, 09.
- Validation evidence quality: strong on USB Serial0 profile, partial on board-specific RS485 deployments.

## Untested or Partial Hardware Validation

- Atomic transactions under sustained hardware stress
- RS485 DE/RE timing across transceiver families and cable lengths
- Endianness mode switching automation in runtime test harnesses
- Direct FC08 request automation and clearDiagnosticsCounters verification
- Alternative Stream transport runtime behavior (non-HardwareSerial)
- Full board matrix (multi-UART MEGA variants, ESP32 variants)

## Operational Rules Found in Field-Like Testing

- Opening COM often resets Arduino boards.
- Wait about 2.5 seconds after connect before first request.
- Keep Serial Monitor closed during Modbus traffic on Serial0.
