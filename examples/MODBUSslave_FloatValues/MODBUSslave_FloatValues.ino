// Modbus RTU Slave - Float Value Example
// Demonstrates setFloat/getFloat across two 16-bit holding registers.

#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

modbusDevice regBank;
modbusSlave slave;

static float temperatureC = 23.5f;
static float pressureBar = 1.20f;
static unsigned long lastUpdateMs = 0;

void setup() {
  regBank.setId(1);

  // Float registers use two adjacent holding registers each.
  // temperatureC -> 40001 (high word), 40002 (low word)
  // pressureBar  -> 40003 (high word), 40004 (low word)
  regBank.add(40001);
  regBank.add(40002);
  regBank.add(40003);
  regBank.add(40004);

  regBank.setFloat(40001, temperatureC);
  regBank.setFloat(40003, pressureBar);

  slave.setDevice(&regBank);
  slave.setPort(Serial);
  slave.setBaud(9600);

  // Optional 32-bit word order configuration for PLC compatibility.
  // slave.configureEndianness(MODBUS_BIG_ENDIAN);
  // slave.configureEndianness(MODBUS_LITTLE_ENDIAN);
  // slave.configureEndianness(MODBUS_BIG_ENDIAN_SWAPPED);

  // Optional RS485 DE/RE control with microsecond precision delays.
  // slave.setTxEnablePin(2, true);
  // slave.setTxEnableDelaysUs(100, 100);
}

void loop() {
  slave.run();

  // Update process values every 500ms and publish as Modbus floats.
  if ((millis() - lastUpdateMs) >= 500) {
    lastUpdateMs = millis();

    temperatureC += 0.1f;
    if (temperatureC > 30.0f) temperatureC = 23.5f;

    pressureBar += 0.02f;
    if (pressureBar > 1.80f) pressureBar = 1.20f;

    regBank.setFloat(40001, temperatureC);
    regBank.setFloat(40003, pressureBar);

    // Optional local read-back example:
    // float t = regBank.getFloat(40001);
    // float p = regBank.getFloat(40003);
    // (void)t;
    // (void)p;
  }
}
