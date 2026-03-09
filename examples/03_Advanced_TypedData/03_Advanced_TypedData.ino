/*******************************************************************************
 * LEVEL 3: ADVANCED - Typed Data (Float, Long, String)
 * 
 * WHAT YOU'LL LEARN:
 * - Using 32-bit floating-point values across two registers
 * - Using 32-bit long integers across two registers
 * - Storing ASCII strings (2 characters per register)
 * - Configuring endianness for multi-vendor compatibility
 * - Advanced data type conversions
 * 
 * WHY USE TYPED HELPERS?
 * Standard Modbus only supports 16-bit registers. For larger data types:
 * - Float (32-bit IEEE-754) requires 2 consecutive registers
 * - Long (32-bit integer) requires 2 consecutive registers
 * - Strings require N/2 registers (2 ASCII chars per register)
 * 
 * This library provides helper functions to pack/unpack these types automatically.
 * 
 * ENDIANNESS MODES:
 * Different PLCs and SCADA systems use different byte orders for 32-bit values:
 * 
 * MODBUS_BIG_ENDIAN (default):
 *   Most common for Modbus RTU
 *   Register order: [High Word][Low Word]
 *   Example: Float 123.45 = [0x42F6][0xE666]
 * 
 * MODBUS_LITTLE_ENDIAN:
 *   Used by some PLCs (Allen-Bradley, etc.)
 *   Register order: [Low Word][High Word]
 *   Example: Float 123.45 = [0xE666][0x42F6]
 * 
 * MODBUS_BIG_ENDIAN_SWAPPED:
 *   Byte-swapped within each register
 *   Used by some Siemens PLCs
 * 
 * HARDWARE REQUIRED:
 * - Arduino UNO, MEGA, or compatible
 * - USB connection for testing
 * 
 * MODBUS SLAVE ADDRESS: 1
 * BAUD RATE: 19200
 * 
 * REGISTER MAP:
 * 
 * FLOAT VALUES (32-bit IEEE-754, 2 registers each):
 *    40001-40002: Temperature (°C)        Example: 23.45°C
 *    40003-40004: Pressure (bar)          Example: 1.23 bar
 *    40005-40006: Flow Rate (L/min)       Example: 12.789 L/min
 *    40007-40008: Humidity (%)            Example: 65.4%
 * 
 * LONG VALUES (32-bit signed integer, 2 registers each):
 *    40011-40012: Total Runtime (seconds) Example: 123456789
 *    40013-40014: Pulse Counter           Example: 987654321
 *    40015-40016: Error Code              Example: -1 or 0x80000000
 * 
 * STRING VALUES (ASCII, 2 chars per register):
 *    40021-40030: Device Name (20 chars)  Example: "PUMP-CTRL-01"
 *    40031-40035: Status Message (10 chars) Example: "RUNNING"
 * 
 ******************************************************************************/

#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

// Global objects
modbusDevice regBank;
modbusSlave slave;

// Process variables for demonstration
float temperature = 23.45;
float pressure = 1.23;
float flowRate = 12.789;
float humidity = 65.4;

long totalRuntime = 0;          // Seconds since start
long pulseCounter = 0;          // Simulated pulse count
long errorCode = 0;             // 0 = no error

char deviceName[21] = "PUMP-CTRL-01";
char statusMessage[11] = "RUNNING";

unsigned long lastUpdate = 0;
unsigned long startTime = 0;

void setup() {
  // Set slave ID
  regBank.setId(1);
  
  // ===== FLOAT REGISTERS =====
  // Each float requires 2 consecutive registers
  regBank.add(40001);  // Temperature (high word)
  regBank.add(40002);  // Temperature (low word)
  regBank.add(40003);  // Pressure (high word)
  regBank.add(40004);  // Pressure (low word)
  regBank.add(40005);  // Flow rate (high word)
  regBank.add(40006);  // Flow rate (low word)
  regBank.add(40007);  // Humidity (high word)
  regBank.add(40008);  // Humidity (low word)
  
  // ===== LONG REGISTERS =====
  // Each long requires 2 consecutive registers
  regBank.add(40011);  // Runtime (high word)
  regBank.add(40012);  // Runtime (low word)
  regBank.add(40013);  // Pulse counter (high word)
  regBank.add(40014);  // Pulse counter (low word)
  regBank.add(40015);  // Error code (high word)
  regBank.add(40016);  // Error code (low word)
  
  // ===== STRING REGISTERS =====
  // Device name: 20 characters = 10 registers (2 chars per register)
  for (word addr = 40021; addr <= 40030; addr++) {
    regBank.add(addr);
  }
  
  // Status message: 10 characters = 5 registers
  for (word addr = 40031; addr <= 40035; addr++) {
    regBank.add(addr);
  }
  
  // ===== WRITE INITIAL VALUES =====
  regBank.setFloat(40001, temperature);
  regBank.setFloat(40003, pressure);
  regBank.setFloat(40005, flowRate);
  regBank.setFloat(40007, humidity);
  
  regBank.setLong(40011, totalRuntime);
  regBank.setLong(40013, pulseCounter);
  regBank.setLong(40015, errorCode);
  
  regBank.setString(40021, deviceName, 10);   // 10 registers for 20 chars
  regBank.setString(40031, statusMessage, 5); // 5 registers for 10 chars
  
  // ===== MODBUS CONFIGURATION =====
  slave.setDevice(&regBank);
  slave.setPort(Serial);
  slave.setProtocol(RTU);  // Default framing mode
  slave.setBaud(19200);
  
  // ===== ENDIANNESS CONFIGURATION =====
  // Choose the endianness mode that matches your SCADA/PLC system
  // Default is MODBUS_BIG_ENDIAN (most common)
  
  // slave.configureEndianness(MODBUS_BIG_ENDIAN);        // Default
  // slave.configureEndianness(MODBUS_LITTLE_ENDIAN);     // Allen-Bradley style
  // slave.configureEndianness(MODBUS_BIG_ENDIAN_SWAPPED); // Some Siemens PLCs
  
  // Example: If your SCADA system shows garbled float values,
  // try different endianness modes until values display correctly
  
  startTime = millis();
}

void loop() {
  // Process Modbus requests
  slave.run();
  
  // Update values every second
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    
    updateProcessValues();
    updateCounters();
    updateStatusMessage();
  }
}

/*******************************************************************************
 * Update Process Values (Float data)
 ******************************************************************************/
void updateProcessValues() {
  // Simulate slowly changing process values
  temperature += 0.1;
  if (temperature > 30.0) temperature = 20.0;
  
  pressure += 0.02;
  if (pressure > 2.0) pressure = 1.0;
  
  flowRate += 0.5;
  if (flowRate > 20.0) flowRate = 10.0;
  
  humidity -= 0.2;
  if (humidity < 50.0) humidity = 70.0;
  
  // Write floats to Modbus registers
  // Each float occupies 2 registers (32 bits)
  regBank.setFloat(40001, temperature);
  regBank.setFloat(40003, pressure);
  regBank.setFloat(40005, flowRate);
  regBank.setFloat(40007, humidity);
  
  // READING BACK FLOATS:
  // You can also read float values if needed
  // float temp = regBank.getFloat(40001);
  // float pres = regBank.getFloat(40003);
}

/*******************************************************************************
 * Update Counters (Long data)
 ******************************************************************************/
void updateCounters() {
  // Calculate total runtime in seconds
  totalRuntime = (millis() - startTime) / 1000;
  
  // Simulate pulse counter incrementing
  pulseCounter += 100;
  if (pulseCounter > 2000000000L) pulseCounter = 0;
  
  // Error code demonstration
  // 0 = no error, negative values indicate error codes
  if (temperature > 28.0) {
    errorCode = -100;  // Example: -100 = high temperature warning
  } else {
    errorCode = 0;     // No error
  }
  
  // Write long values to Modbus registers
  // Each long occupies 2 registers (32 bits signed)
  regBank.setLong(40011, totalRuntime);
  regBank.setLong(40013, pulseCounter);
  regBank.setLong(40015, errorCode);
  
  // READING BACK LONGS:
  // long runtime = regBank.getLong(40011);
  // long pulses = regBank.getLong(40013);
}

/*******************************************************************************
 * Update Status Message (String data)
 ******************************************************************************/
void updateStatusMessage() {
  // Change status message based on conditions
  if (errorCode != 0) {
    strcpy(statusMessage, "ERROR");
  } else if (temperature > 25.0) {
    strcpy(statusMessage, "HOT");
  } else {
    strcpy(statusMessage, "RUNNING");
  }
  
  // Write string to Modbus registers
  // 5 registers can hold up to 10 ASCII characters
  regBank.setString(40031, statusMessage, 5);
  
  // READING BACK STRINGS:
  // char buffer[11];  // 10 chars + null terminator
  // regBank.getString(40031, buffer, sizeof(buffer), 5);
  
  // Note: Device name (40021-40030) is constant, so we don't update it
  // But you could allow the master to write it via FC16 (Write Multiple Registers)
}

/*******************************************************************************
 * TESTING GUIDE
 * 
 * 1. READ FLOAT VALUES:
 *    Configure your master tool to read registers as 32-bit float:
 *    - Read address 40001, count 2 (or use float read if supported)
 *    - Should display: ~23.xx°C (slowly incrementing)
 * 
 * 2. VERIFY ENDIANNESS:
 *    If float values appear as garbage (e.g., 3.2e+38), try different modes:
 *    - Uncomment slave.configureEndianness(MODBUS_LITTLE_ENDIAN);
 *    - Re-upload and test again
 * 
 * 3. READ LONG VALUES:
 *    Configure your master to read as 32-bit signed integer:
 *    - Read address 40011, count 2
 *    - Should show runtime in seconds (incrementing)
 * 
 * 4. READ STRING VALUES:
 *    Configure your master to read as ASCII string:
 *    - Read address 40021, count 10 (20 characters)
 *    - Should display: "PUMP-CTRL-01"
 *    - Read address 40031, count 5 (10 characters)
 *    - Should display: "RUNNING" or "HOT" or "ERROR"
 * 
 * 5. WRITE STRING TEST:
 *    Write new device name using FC16 (Write Multiple Registers):
 *    - Write address 40021, count 10
 *    - Send ASCII data: "NEW-NAME-123"
 *    - Read back to verify
 * 
 * 6. DATA TYPE RANGES:
 *    Float:  ±3.4e±38 (7 decimal digits precision)
 *    Long:   -2,147,483,648 to 2,147,483,647
 *    String: Limited only by number of registers allocated
 * 
 ******************************************************************************/

/*******************************************************************************
 * COMMON ISSUES AND SOLUTIONS
 * 
 * PROBLEM: Float values show as huge numbers or garbage
 * SOLUTION: Your master is using different endianness.
 *           Try: slave.configureEndianness(MODBUS_LITTLE_ENDIAN);
 * 
 * PROBLEM: String shows garbled characters
 * SOLUTION: Ensure your master is interpreting as ASCII, not UTF-16.
 *           Some tools require "High byte first" or "Big endian" setting.
 * 
 * PROBLEM: Long values are negative when they should be positive
 * SOLUTION: Endianness issue. Try different modes.
 *           Or: Use unsigned long (but setLong/getLong use signed)
 * 
 * PROBLEM: Reading single register from float pair shows strange value
 * SOLUTION: Always read BOTH registers of a float/long pair together.
 *           Reading just one register gives you half of the data.
 * 
 ******************************************************************************/
