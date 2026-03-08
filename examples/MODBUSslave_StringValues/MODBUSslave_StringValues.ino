// Modbus RTU Slave - ASCII String Example
// Demonstrates 2 ASCII characters per 16-bit holding register.

#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

modbusDevice regBank;
modbusSlave slave;

// String area in holding registers:
// 40020..40029 -> up to 20 ASCII characters (2 chars per register)
static const word STRING_BASE = 40020;
static const word STRING_REG_COUNT = 10;

char rxBuffer[21];
unsigned long lastScanMs = 0;

void setup() {
  regBank.setId(1);

  for (word i = 0; i < STRING_REG_COUNT; i++) {
    regBank.add(STRING_BASE + i);
    regBank.set(STRING_BASE + i, 0);
  }

  // Write initial text into registers: "PUMP-OK"
  regBank.setString(STRING_BASE, "PUMP-OK", STRING_REG_COUNT);

  slave.setDevice(&regBank);
  slave.setPort(Serial);
  slave.setBaud(9600);
}

void loop() {
  slave.run();

  // Optional local read-back every second.
  if ((millis() - lastScanMs) >= 1000) {
    lastScanMs = millis();

    // Read string from registers into local buffer.
    regBank.getString(STRING_BASE, rxBuffer, sizeof(rxBuffer), STRING_REG_COUNT);

    // Example dynamic update:
    // if (strcmp(rxBuffer, "PUMP-FAIL") == 0) {
    //   regBank.setString(STRING_BASE, "ALARM-ACK", STRING_REG_COUNT);
    // }
  }
}
