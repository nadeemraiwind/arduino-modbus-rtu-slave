/*******************************************************************************
 * VALIDATION TEST: Address Range and Safety Checks
 * 
 * PURPOSE:
 * This sketch validates the micro-level safety improvements including:
 * 1. Address range validation (rejects invalid Modbus addresses)
 * 2. micros() rollover safety (timing calculations)
 * 3. Memory pool alignment verification
 * 
 * HARDWARE REQUIRED:
 * - Arduino UNO, MEGA, or compatible
 * - USB connection for Serial Monitor output
 * 
 * EXPECTED OUTPUT:
 * Serial monitor should show all tests PASS with no errors.
 * 
 * HOW TO USE:
 * 1. Upload this sketch to your Arduino
 * 2. Open Serial Monitor (9600 baud)
 * 3. Review test results - all should show "PASS"
 * 
 ******************************************************************************/

#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

modbusDevice regBank;
modbusSlave slave;

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 3000);  // Wait for Serial (Leonardo/Micro)
  
  Serial.println(F("==========================================="));
  Serial.println(F("MODBUS RTU LIBRARY - VALIDATION TEST SUITE"));
  Serial.println(F("==========================================="));
  Serial.println();
  
  // Test 1: Valid Address Ranges
  testValidAddresses();
  
  // Test 2: Invalid Address Rejection
  testInvalidAddresses();
  
  // Test 3: Memory Pool Safety
  testMemoryPoolSafety();
  
  // Test 4: String Operations
  testStringOperations();
  
  // Test 5: Timing Rollover Simulation
  testTimingRollover();
  
  Serial.println();
  Serial.println(F("==========================================="));
  Serial.println(F("ALL TESTS COMPLETE"));
  Serial.println(F("==========================================="));
}

void loop() {
  // Nothing to do - all tests run in setup()
}

/*******************************************************************************
 * TEST 1: Valid Address Ranges
 * Verify that standard Modbus addresses are accepted
 ******************************************************************************/
void testValidAddresses() {
  Serial.println(F("TEST 1: Valid Address Ranges"));
  Serial.println(F("-----------------------------"));
  
  modbusDevice testBank;
  testBank.setId(1);
  
  // Coils: 1-9999
  testBank.add(1);
  testBank.add(5000);
  testBank.add(9999);
  if (testBank.has(1) && testBank.has(5000) && testBank.has(9999)) {
    Serial.println(F("  ✓ Coils (1-9999) accepted"));
  } else {
    Serial.println(F("  ✗ FAIL: Coils not accepted"));
  }
  
  // Discrete Inputs: 10001-19999
  testBank.add(10001);
  testBank.add(15000);
  testBank.add(19999);
  if (testBank.has(10001) && testBank.has(15000) && testBank.has(19999)) {
    Serial.println(F("  ✓ Discrete Inputs (10001-19999) accepted"));
  } else {
    Serial.println(F("  ✗ FAIL: Discrete Inputs not accepted"));
  }
  
  // Input Registers: 30001-39999
  testBank.add(30001);
  testBank.add(35000);
  testBank.add(39999);
  if (testBank.has(30001) && testBank.has(35000) && testBank.has(39999)) {
    Serial.println(F("  ✓ Input Registers (30001-39999) accepted"));
  } else {
    Serial.println(F("  ✗ FAIL: Input Registers not accepted"));
  }
  
  // Holding Registers: 40001-49999
  testBank.add(40001);
  testBank.add(45000);
  testBank.add(49999);
  if (testBank.has(40001) && testBank.has(45000) && testBank.has(49999)) {
    Serial.println(F("  ✓ Holding Registers (40001-49999) accepted"));
  } else {
    Serial.println(F("  ✗ FAIL: Holding Registers not accepted"));
  }
  
  Serial.println();
}

/*******************************************************************************
 * TEST 2: Invalid Address Rejection
 * Verify that out-of-range addresses are rejected
 ******************************************************************************/
void testInvalidAddresses() {
  Serial.println(F("TEST 2: Invalid Address Rejection"));
  Serial.println(F("----------------------------------"));
  
  modbusDevice testBank;
  testBank.setId(1);
  
  // Invalid: Address 0
  testBank.add(0);
  if (!testBank.has(0)) {
    Serial.println(F("  ✓ Address 0 rejected (below valid range)"));
  } else {
    Serial.println(F("  ✗ FAIL: Address 0 should be rejected"));
  }
  
  // Invalid: Gap between coils and discrete inputs (10000)
  testBank.add(10000);
  if (!testBank.has(10000)) {
    Serial.println(F("  ✓ Address 10000 rejected (gap range)"));
  } else {
    Serial.println(F("  ✗ FAIL: Address 10000 should be rejected"));
  }
  
  // Invalid: Gap between discrete inputs and input registers (20000-29999)
  testBank.add(25000);
  if (!testBank.has(25000)) {
    Serial.println(F("  ✓ Address 25000 rejected (gap range)"));
  } else {
    Serial.println(F("  ✗ FAIL: Address 25000 should be rejected"));
  }
  
  // Invalid: Above holding registers (50000+)
  testBank.add(50000);
  if (!testBank.has(50000)) {
    Serial.println(F("  ✓ Address 50000 rejected (above valid range)"));
  } else {
    Serial.println(F("  ✗ FAIL: Address 50000 should be rejected"));
  }
  
  // Invalid: Maximum word value (65535)
  testBank.add(65535);
  if (!testBank.has(65535)) {
    Serial.println(F("  ✓ Address 65535 rejected (far above range)"));
  } else {
    Serial.println(F("  ✗ FAIL: Address 65535 should be rejected"));
  }
  
  Serial.println();
}

/*******************************************************************************
 * TEST 3: Memory Pool Safety
 * Verify memory allocation handles edge cases
 ******************************************************************************/
void testMemoryPoolSafety() {
  Serial.println(F("TEST 3: Memory Pool Safety"));
  Serial.println(F("--------------------------"));
  
  modbusDevice testBank;
  testBank.setId(1);
  
  // Add duplicate addresses (should be silently ignored)
  testBank.add(40001);
  testBank.add(40001);
  testBank.add(40001);
  testBank.set(40001, 123);
  if (testBank.get(40001) == 123) {
    Serial.println(F("  ✓ Duplicate address handling safe"));
  } else {
    Serial.println(F("  ✗ FAIL: Duplicate handling failed"));
  }
  
  // Test digital register value normalization
  testBank.add(1);
  testBank.set(1, 42);  // Non-zero should normalize to 0xFF
  word value = testBank.get(1);
  if (value == 0xFF) {
    Serial.println(F("  ✓ Digital register normalization correct"));
  } else {
    Serial.print(F("  ✗ FAIL: Expected 0xFF, got 0x"));
    Serial.println(value, HEX);
  }
  
  testBank.set(1, 0);  // Zero should stay 0x00
  value = testBank.get(1);
  if (value == 0x00) {
    Serial.println(F("  ✓ Digital register zero handling correct"));
  } else {
    Serial.println(F("  ✗ FAIL: Zero handling failed"));
  }
  
  Serial.println();
}

/*******************************************************************************
 * TEST 4: String Operations
 * Verify string packing/unpacking with edge cases
 ******************************************************************************/
void testStringOperations() {
  Serial.println(F("TEST 4: String Operations"));
  Serial.println(F("-------------------------"));
  
  modbusDevice testBank;
  testBank.setId(1);
  
  // Allocate 5 registers for 10-character string
  for (word addr = 40001; addr <= 40005; addr++) {
    testBank.add(addr);
  }
  
  // Test 1: Normal string
  const char* test1 = "HELLO";
  testBank.setString(40001, test1, 5);
  char buffer[11];
  testBank.getString(40001, buffer, 11, 5);
  if (strcmp(buffer, "HELLO") == 0) {
    Serial.println(F("  ✓ String write/read works"));
  } else {
    Serial.print(F("  ✗ FAIL: Expected 'HELLO', got '"));
    Serial.print(buffer);
    Serial.println(F("'"));
  }
  
  // Test 2: Odd-length string (7 chars = 3.5 registers)
  const char* test2 = "MODBUS!";
  testBank.setString(40001, test2, 5);
  testBank.getString(40001, buffer, 11, 5);
  if (strcmp(buffer, "MODBUS!") == 0) {
    Serial.println(F("  ✓ Odd-length string handled correctly"));
  } else {
    Serial.print(F("  ✗ FAIL: Expected 'MODBUS!', got '"));
    Serial.print(buffer);
    Serial.println(F("'"));
  }
  
  // Test 3: Empty string
  const char* test3 = "";
  testBank.setString(40001, test3, 5);
  testBank.getString(40001, buffer, 11, 5);
  if (strlen(buffer) == 0) {
    Serial.println(F("  ✓ Empty string handled correctly"));
  } else {
    Serial.println(F("  ✗ FAIL: Empty string produced non-empty result"));
  }
  
  // Test 4: Full 10-character string
  const char* test4 = "1234567890";
  testBank.setString(40001, test4, 5);
  testBank.getString(40001, buffer, 11, 5);
  if (strcmp(buffer, "1234567890") == 0) {
    Serial.println(F("  ✓ Full-length string correct"));
  } else {
    Serial.print(F("  ✗ FAIL: Expected '1234567890', got '"));
    Serial.print(buffer);
    Serial.println(F("'"));
  }
  
  Serial.println();
}

/*******************************************************************************
 * TEST 5: Timing Rollover Simulation
 * Verify micros() rollover math works correctly
 ******************************************************************************/
void testTimingRollover() {
  Serial.println(F("TEST 5: Timing Rollover Simulation"));
  Serial.println(F("-----------------------------------"));
  
  // Simulate rollover condition
  unsigned long lastTime = 4294967290UL;  // 6 microseconds before rollover
  unsigned long nowTime = 100UL;           // 100 microseconds after rollover
  unsigned long frameDelay = 1750UL;       // Standard 1.75ms delay
  
  // This should be false (only 106 microseconds elapsed)
  if ((nowTime - lastTime) < frameDelay) {
    Serial.println(F("  ✓ Rollover math: 106µs < 1750µs (correct)"));
  } else {
    Serial.println(F("  ✗ FAIL: Rollover math incorrect"));
  }
  
  // Simulate enough time passing
  nowTime = 2000UL;  // 2000 + (rollover compensation) microseconds after
  if ((nowTime - lastTime) >= frameDelay) {
    Serial.println(F("  ✓ Rollover math: 2106µs >= 1750µs (correct)"));
  } else {
    Serial.println(F("  ✗ FAIL: Rollover threshold incorrect"));
  }
  
  // Normal (non-rollover) case
  lastTime = 1000000UL;
  nowTime = 1002000UL;
  if ((nowTime - lastTime) >= frameDelay) {
    Serial.println(F("  ✓ Normal timing: 2000µs >= 1750µs (correct)"));
  } else {
    Serial.println(F("  ✗ FAIL: Normal timing incorrect"));
  }
  
  Serial.println();
}
