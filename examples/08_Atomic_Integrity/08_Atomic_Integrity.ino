/*******************************************************************************
 * LEVEL 8: ATOMIC INTEGRITY - Multi-Register Safety Under Load
 *
 * WHAT YOU'LL LEARN:
 * - Why 32-bit values (float/long) can tear across two 16-bit Modbus registers
 * - How atomicBegin()/atomicEnd() protects multi-register updates
 * - How to expose integrity counters to prove updates remain consistent
 *
 * USE CASE:
 * - In busy loop logic, a master can poll while firmware is updating a 32-bit value.
 * - Without atomic sections, the master might read one old word and one new word.
 * - This sketch demonstrates the safe pattern for industrial-grade data integrity.
 *
 * MODBUS SETTINGS:
 * - Slave ID: 1
 * - Baud: 19200
 * - Framing: RTU
 *
 * REGISTER MAP:
 * - 40001-40002: processValue (float)
 * - 40003-40004: calculatedPower (float)
 * - 40011-40012: updateSequence (long, increments every cycle)
 * - 40013-40014: cycleTimeUs (long)
 * - 40020: atomicWriteCount (word)
 * - 40021: overloadFlag (0/1)
 *
 * SERIAL PROFILE NOTES:
 * - Default validated profile uses slave.setPort(Serial).
 * - Keep Serial Monitor closed while Modbus is active on Serial.
 * - If you need USB debug logs, move Modbus to another UART on multi-UART boards.
 ******************************************************************************/

/**
 * @example 08_Atomic_Integrity.ino
 * Atomic update pattern to prevent torn 32-bit Modbus reads.
 */

#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

modbusDevice regBank;
modbusSlave slave;

float processValue = 10.0f;
float calculatedPower = 0.0f;
long updateSequence = 0;
long cycleTimeUs = 0;
word atomicWriteCount = 0;
bool overloadFlag = false;

unsigned long lastUpdateMs = 0;

void setup() {
  regBank.setId(1);

  // Float pairs
  regBank.add(40001);
  regBank.add(40002);
  regBank.add(40003);
  regBank.add(40004);

  // Long pairs
  regBank.add(40011);
  regBank.add(40012);
  regBank.add(40013);
  regBank.add(40014);

  // Status words
  regBank.add(40020);
  regBank.add(40021);

  regBank.setFloat(40001, processValue);
  regBank.setFloat(40003, calculatedPower);
  regBank.setLong(40011, updateSequence);
  regBank.setLong(40013, cycleTimeUs);
  regBank.set(40020, atomicWriteCount);
  regBank.set(40021, overloadFlag ? 1 : 0);

  slave.setDevice(&regBank);
  slave.setPort(Serial);
  slave.setProtocol(RTU);
  slave.setBaud(19200);
  slave.configureEndianness(MODBUS_BIG_ENDIAN);
}

void loop() {
  slave.run();

  if (millis() - lastUpdateMs < 200) {
    return;
  }
  lastUpdateMs = millis();

  unsigned long t0 = micros();

  // Simulate work and state transitions.
  processValue += 0.37f;
  if (processValue > 120.0f) {
    processValue = 10.0f;
  }

  calculatedPower = processValue * 1.618f;
  overloadFlag = (calculatedPower > 150.0f);
  updateSequence++;

  // Critical section: update all multi-register values atomically.
  regBank.atomicBegin();
  regBank.setFloat(40001, processValue);
  regBank.setFloat(40003, calculatedPower);
  regBank.setLong(40011, updateSequence);
  regBank.atomicEnd();

  cycleTimeUs = (long)(micros() - t0);

  // Keep metadata coherent as well.
  regBank.atomicBegin();
  regBank.setLong(40013, cycleTimeUs);
  regBank.set(40021, overloadFlag ? 1 : 0);
  atomicWriteCount++;
  regBank.set(40020, atomicWriteCount);
  regBank.atomicEnd();
}
