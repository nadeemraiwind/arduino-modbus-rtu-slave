/*******************************************************************************
 * LEVEL 6: VALIDATION TESTS - Modbus-Readable Library Safety Checks
 *
 * WHAT YOU'LL LEARN:
 * - How to verify library behavior without relying on Serial Monitor output
 * - Address range validation for standard Modbus spaces
 * - Duplicate-register safety and digital normalization behavior
 * - String helper coverage with edge cases
 * - Sentinel-safe reads with tryGet()
 * - Protocol selection checks with setProtocol()/getProtocol()
 *
 * WHY THIS EXAMPLE EXISTS:
 * - Earlier development versions printed PASS/FAIL text to Serial Monitor.
 * - That conflicts with the validated Serial0 Modbus workflow because Serial0 is
 *   reserved for Modbus traffic when `slave.setPort(Serial)` is used.
 * - This public example now exposes validation results through holding registers,
 *   so you can inspect results from any Modbus master.
 * - It is intentionally a self-test/regression sketch, not the first sketch to
 *   learn Modbus concepts. Start with Level 1 for initial onboarding.
 *
 * VALIDATED WORKFLOW:
 * - Default validated profile: `slave.setPort(Serial)` using USB Serial0.
 * - Keep Serial Monitor closed and do not use Serial.print() in that profile.
 * - Wait about 2 to 3 seconds after COM open before polling because Arduino USB
 *   boards commonly reset when the serial port opens.
 *
 * HOW TO CHANGE SERIAL PORTS AND ENABLE DEBUG:
 * - Default: keep Modbus on `Serial` for the simplest validation workflow.
 * - MEGA / multi-UART boards: move Modbus to `Serial1`, `Serial2`, or `Serial3`
 *   if you want USB `Serial` available for extra debug prints.
 * - Single-UART boards should keep debugging register-based instead of using
 *   Serial Monitor while Modbus is active.
 *
 * RESULT REGISTER MAP:
 *   40001: Suite complete flag (0 = running/not started, 1 = complete)
 *   40002: Passed test count
 *   40003: Failed test count
 *   40004: Current/last test number
 *   40005: Last error code (0 = no error)
 *   40006: Active protocol mode (0 = RTU, 1 = ASCII)
 *   40011-40017: Per-test status (0 = not run, 1 = pass, 2 = fail)
 *
 * DEFAULT SETTINGS:
 * - Slave ID: 1
 * - Baud: 9600
 * - Framing: RTU
 *
 ******************************************************************************/

/**
 * @example 06_Validation_Tests.ino
 * Register-exposed validation suite for regression and deployment checks.
 */

#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

#define SLAVE_ID               1
#define MODBUS_BAUD            9600

#define REG_SUITE_COMPLETE     40001
#define REG_TESTS_PASSED       40002
#define REG_TESTS_FAILED       40003
#define REG_ACTIVE_TEST        40004
#define REG_LAST_ERROR         40005
#define REG_PROTOCOL_MODE      40006
#define REG_TEST_STATUS_BASE   40011
#define REG_TEST_STATUS_COUNT  7

#define TEST_STATUS_NOT_RUN    0
#define TEST_STATUS_PASS       1
#define TEST_STATUS_FAIL       2

#define ERR_NONE               0
#define ERR_VALID_RANGE        101
#define ERR_INVALID_RANGE      102
#define ERR_DUPLICATE_SAFE     103
#define ERR_DIGITAL_NORMALIZE  104
#define ERR_DIGITAL_ZERO       105
#define ERR_STRING_BASIC       106
#define ERR_STRING_ODD         107
#define ERR_STRING_EMPTY       108
#define ERR_STRING_FULL        109
#define ERR_ROLLOVER_LT        110
#define ERR_ROLLOVER_GTE       111
#define ERR_TIMING_NORMAL      112
#define ERR_TRYGET_VALID       113
#define ERR_TRYGET_MISSING     114
#define ERR_PROTOCOL_ASCII     115
#define ERR_PROTOCOL_RTU       116

modbusDevice regBank;
modbusSlave slave;

static void setupResultRegisters();
static void resetResultRegisters();
static void beginTest(byte testNumber);
static void finishTest(byte testNumber, bool passed, word errorCode);
static void runValidationSuite();

static bool testValidAddresses(word *errorCode);
static bool testInvalidAddresses(word *errorCode);
static bool testMemoryPoolSafety(word *errorCode);
static bool testStringOperations(word *errorCode);
static bool testTimingRollover(word *errorCode);
static bool testTryGetSafety(word *errorCode);
static bool testProtocolSelection(word *errorCode);

void setup() {
  regBank.setId(SLAVE_ID);
  setupResultRegisters();
  resetResultRegisters();

  slave.setDevice(&regBank);
  slave.setPort(Serial);  // Validated USB Serial0 profile. Move Modbus to another UART before enabling Serial debug.
  slave.setProtocol(RTU);
  slave.setBaud(MODBUS_BAUD);

  runValidationSuite();
  regBank.set(REG_SUITE_COMPLETE, 1);
  regBank.set(REG_ACTIVE_TEST, 0);
  regBank.set(REG_PROTOCOL_MODE, slave.getProtocol());
}

void loop() {
  slave.run();
}

static void setupResultRegisters() {
  for (word addr = REG_SUITE_COMPLETE; addr <= REG_PROTOCOL_MODE; addr++) {
    regBank.add(addr);
  }

  for (word offset = 0; offset < REG_TEST_STATUS_COUNT; offset++) {
    regBank.add((word)(REG_TEST_STATUS_BASE + offset));
  }
}

static void resetResultRegisters() {
  regBank.set(REG_SUITE_COMPLETE, 0);
  regBank.set(REG_TESTS_PASSED, 0);
  regBank.set(REG_TESTS_FAILED, 0);
  regBank.set(REG_ACTIVE_TEST, 0);
  regBank.set(REG_LAST_ERROR, 0);
  regBank.set(REG_PROTOCOL_MODE, RTU);

  for (word offset = 0; offset < REG_TEST_STATUS_COUNT; offset++) {
    regBank.set((word)(REG_TEST_STATUS_BASE + offset), TEST_STATUS_NOT_RUN);
  }
}

static void beginTest(byte testNumber) {
  regBank.set(REG_ACTIVE_TEST, testNumber);
  regBank.set(REG_LAST_ERROR, ERR_NONE);
}

static void finishTest(byte testNumber, bool passed, word errorCode) {
  regBank.set((word)(REG_TEST_STATUS_BASE + (testNumber - 1)),
              passed ? TEST_STATUS_PASS : TEST_STATUS_FAIL);

  if (passed) {
    regBank.set(REG_TESTS_PASSED, (word)(regBank.get(REG_TESTS_PASSED) + 1));
    regBank.set(REG_LAST_ERROR, ERR_NONE);
  } else {
    regBank.set(REG_TESTS_FAILED, (word)(regBank.get(REG_TESTS_FAILED) + 1));
    regBank.set(REG_LAST_ERROR, errorCode);
  }
}

static void runValidationSuite() {
  word errorCode = ERR_NONE;
  bool passed = false;

  beginTest(1);
  passed = testValidAddresses(&errorCode);
  finishTest(1, passed, errorCode);

  beginTest(2);
  passed = testInvalidAddresses(&errorCode);
  finishTest(2, passed, errorCode);

  beginTest(3);
  passed = testMemoryPoolSafety(&errorCode);
  finishTest(3, passed, errorCode);

  beginTest(4);
  passed = testStringOperations(&errorCode);
  finishTest(4, passed, errorCode);

  beginTest(5);
  passed = testTimingRollover(&errorCode);
  finishTest(5, passed, errorCode);

  beginTest(6);
  passed = testTryGetSafety(&errorCode);
  finishTest(6, passed, errorCode);

  beginTest(7);
  passed = testProtocolSelection(&errorCode);
  finishTest(7, passed, errorCode);
}

static bool testValidAddresses(word *errorCode) {
  modbusDevice testBank;
  testBank.setId(1);

  testBank.add(1);
  testBank.add(5000);
  testBank.add(9999);
  testBank.add(10001);
  testBank.add(15000);
  testBank.add(19999);
  testBank.add(30001);
  testBank.add(35000);
  testBank.add(39999);
  testBank.add(40001);
  testBank.add(45000);
  testBank.add(49999);

  if (!testBank.has(1) || !testBank.has(5000) || !testBank.has(9999) ||
      !testBank.has(10001) || !testBank.has(15000) || !testBank.has(19999) ||
      !testBank.has(30001) || !testBank.has(35000) || !testBank.has(39999) ||
      !testBank.has(40001) || !testBank.has(45000) || !testBank.has(49999)) {
    *errorCode = ERR_VALID_RANGE;
    return false;
  }

  *errorCode = ERR_NONE;
  return true;
}

static bool testInvalidAddresses(word *errorCode) {
  modbusDevice testBank;
  testBank.setId(1);

  testBank.add(0);
  testBank.add(10000);
  testBank.add(25000);
  testBank.add(50000);
  testBank.add(65535);

  if (testBank.has(0) || testBank.has(10000) || testBank.has(25000) ||
      testBank.has(50000) || testBank.has(65535)) {
    *errorCode = ERR_INVALID_RANGE;
    return false;
  }

  *errorCode = ERR_NONE;
  return true;
}

static bool testMemoryPoolSafety(word *errorCode) {
  modbusDevice testBank;
  testBank.setId(1);

  testBank.add(40001);
  testBank.add(40001);
  testBank.add(40001);
  testBank.set(40001, 123);
  if (testBank.get(40001) != 123) {
    *errorCode = ERR_DUPLICATE_SAFE;
    return false;
  }

  testBank.add(1);
  testBank.set(1, 42);
  if (testBank.get(1) != 0xFF) {
    *errorCode = ERR_DIGITAL_NORMALIZE;
    return false;
  }

  testBank.set(1, 0);
  if (testBank.get(1) != 0x00) {
    *errorCode = ERR_DIGITAL_ZERO;
    return false;
  }

  *errorCode = ERR_NONE;
  return true;
}

static bool testStringOperations(word *errorCode) {
  modbusDevice testBank;
  char buffer[11];

  testBank.setId(1);
  for (word addr = 40001; addr <= 40005; addr++) {
    testBank.add(addr);
  }

  testBank.setString(40001, "HELLO", 5);
  testBank.getString(40001, buffer, sizeof(buffer), 5);
  if (strcmp(buffer, "HELLO") != 0) {
    *errorCode = ERR_STRING_BASIC;
    return false;
  }

  testBank.setString(40001, "MODBUS!", 5);
  testBank.getString(40001, buffer, sizeof(buffer), 5);
  if (strcmp(buffer, "MODBUS!") != 0) {
    *errorCode = ERR_STRING_ODD;
    return false;
  }

  testBank.setString(40001, "", 5);
  testBank.getString(40001, buffer, sizeof(buffer), 5);
  if (strlen(buffer) != 0) {
    *errorCode = ERR_STRING_EMPTY;
    return false;
  }

  testBank.setString(40001, "1234567890", 5);
  testBank.getString(40001, buffer, sizeof(buffer), 5);
  if (strcmp(buffer, "1234567890") != 0) {
    *errorCode = ERR_STRING_FULL;
    return false;
  }

  *errorCode = ERR_NONE;
  return true;
}

static bool testTimingRollover(word *errorCode) {
  unsigned long lastTime = 4294967290UL;
  unsigned long nowTime = 100UL;
  unsigned long frameDelay = 1750UL;

  if ((nowTime - lastTime) >= frameDelay) {
    *errorCode = ERR_ROLLOVER_LT;
    return false;
  }

  nowTime = 2000UL;
  if ((nowTime - lastTime) < frameDelay) {
    *errorCode = ERR_ROLLOVER_GTE;
    return false;
  }

  lastTime = 1000000UL;
  nowTime = 1002000UL;
  if ((nowTime - lastTime) < frameDelay) {
    *errorCode = ERR_TIMING_NORMAL;
    return false;
  }

  *errorCode = ERR_NONE;
  return true;
}

static bool testTryGetSafety(word *errorCode) {
  modbusDevice testBank;
  word value = 0;

  testBank.setId(1);
  testBank.add(40050);
  testBank.set(40050, 0xFFFF);

  if (!testBank.tryGet(40050, &value) || value != 0xFFFF) {
    *errorCode = ERR_TRYGET_VALID;
    return false;
  }

  if (testBank.tryGet(40051, &value)) {
    *errorCode = ERR_TRYGET_MISSING;
    return false;
  }

  *errorCode = ERR_NONE;
  return true;
}

static bool testProtocolSelection(word *errorCode) {
  modbusSlave testSlave;

  testSlave.setProtocol(ASCII);
  if (testSlave.getProtocol() != ASCII) {
    *errorCode = ERR_PROTOCOL_ASCII;
    return false;
  }

  testSlave.setProtocol(RTU);
  if (testSlave.getProtocol() != RTU) {
    *errorCode = ERR_PROTOCOL_RTU;
    return false;
  }

  *errorCode = ERR_NONE;
  return true;
}

/*******************************************************************************
 * QUICK TEST GUIDE
 *
 * 1. Upload this sketch.
 * 2. Connect a Modbus RTU master to the board COM port at 9600 8N1, slave ID 1.
 * 3. Wait about 2 to 3 seconds after connect.
 * 4. Read holding registers 40001-40006 and 40011-40017.
 * 5. Expected healthy summary:
 *    - 40001 = 1 (suite complete)
 *    - 40002 = 7 (tests passed)
 *    - 40003 = 0 (tests failed)
 *    - 40005 = 0 (no error)
 *    - 40011-40017 = 1 for all seven test slots
 *
 ******************************************************************************/
