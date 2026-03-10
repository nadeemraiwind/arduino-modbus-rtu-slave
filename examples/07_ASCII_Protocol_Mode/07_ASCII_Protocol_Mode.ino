/*******************************************************************************
 * LEVEL 7: PROTOCOL MODE - Modbus ASCII + Safe Register Reads
 *
 * WHAT YOU'LL LEARN:
 * - How to switch the slave from RTU to ASCII framing
 * - How to use tryGet() to avoid 0xFFFF sentinel ambiguity
 * - How to install a stream baud handler for non-HardwareSerial transports
 *
 * ASCII FRAME FORMAT:
 *   :[ADDR][FUNC][DATA...][LRC]\r\n
 *
 * Notes:
 * - RTU remains the default mode for backwards compatibility.
 * - ASCII is useful with text-oriented links and some legacy gateways.
 * 
 * VALIDATED WORKFLOW:
 * - This example was validated with a Modbus ASCII master on USB `Serial`.
 * - Keep Serial Monitor closed while ASCII traffic is active on `Serial`.
 * - If the board resets when the COM port opens, wait about 2 to 3 seconds before polling.
 * 
 * HOW TO CHANGE SERIAL PORTS AND ENABLE DEBUG:
 * - Default: keep Modbus on `Serial` for the simplest ASCII test setup.
 * - MEGA / multi-UART boards: move Modbus to `Serial1`, `Serial2`, or `Serial3`
 *   if you want USB `Serial` available for debug output.
 * - If Modbus stays on `Serial`, do not call `Serial.print()` while the master is active.
 *
 ******************************************************************************/

/**
 * @example 07_ASCII_Protocol_Mode.ino
 * Modbus ASCII framing mode with safe register-read patterns.
 */

#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

modbusDevice regBank;
modbusSlave slave;

// Example callback used when you bind a non-HardwareSerial Stream transport.
// For plain Serial/Serial1 this callback is not needed.
void onStreamBaudChange(word baud, Stream *port) {
  (void)baud;
  (void)port;
  // Implement stream-specific reconfiguration here when needed.
}

void setup() {
  regBank.setId(1);

  // Coil, input register, and holding register demo map
  regBank.add(1);
  regBank.add(30001);
  regBank.add(40001);

  regBank.set(1, 0);
  regBank.set(30001, 250);   // 25.0 C x10
  regBank.set(40001, 1000);  // Setpoint

  slave.setDevice(&regBank);
  slave.setPort(Serial);  // Validated USB ASCII profile. Move Modbus to another UART before enabling Serial debug.

  // New API: optional handler for generic Stream-based transports.
  slave.setStreamBaudHandler(onStreamBaudChange);

  // New API: switch framing mode.
  // Use RTU for binary framing, ASCII for :HEX...CRLF framing.
  slave.setProtocol(ASCII);
  slave.setBaud(9600);
}

void loop() {
  slave.run();

  // New API: safe read that distinguishes "missing" from valid 0xFFFF payloads.
  word value = 0;
  if (regBank.tryGet(40001, &value)) {
    // Simple demo behavior: mirror holding register LSB to coil.
    regBank.set(1, (value & 0x0001) ? 1 : 0);
  }

  // Simulate changing process data.
  static unsigned long lastTick = 0;
  if (millis() - lastTick >= 1000) {
    lastTick = millis();

    word temp = regBank.get(30001);
    temp++;
    if (temp > 350) temp = 200;
    regBank.set(30001, temp);
  }
}
