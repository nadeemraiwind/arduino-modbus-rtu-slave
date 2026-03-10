/*******************************************************************************
 * LEVEL 4: EXPERT - Callbacks, Diagnostics, and Custom Functions
 * 
 * WHAT YOU'LL LEARN:
 * - Using onRead() callbacks for dynamic value generation
 * - Using onWrite() callbacks to react immediately to master writes
 * - Using onUnknownFunction() for custom/vendor-specific function codes
 * - FC08 Diagnostics function (counters and subfunctions)
 * - Diagnostic counters: bus messages, errors, slave messages
 * - Event logging and condition monitoring
 * 
 * USE CASES FOR CALLBACKS:
 * 
 * onRead():
 *   - Generate values on-demand (save RAM by not storing static data)
 *   - Trigger sensor readings only when requested
 *   - Log master read access for security/audit
 *   - Implement calculated/derived values
 * 
 * onWrite():
 *   - React immediately to critical commands
 *   - Validate written values before accepting
 *   - Trigger actions (start motor, open valve, etc.)
 *   - Mirror values to other registers
 *   - Log configuration changes
 * 
 * onUnknownFunction():
 *   - Implement vendor-specific function codes (FC65-127)
 *   - Custom diagnostics or maintenance functions
 *   - Protocol extensions for specialized equipment
 * 
 * FC08 DIAGNOSTICS:
 * Standard Modbus diagnostics function with subfunctions:
 *   0x0000: Return Query Data (echo test)
 *   0x000B: Get Bus Message Count
 *   0x000C: Get Bus Communication Error Count
 *   0x000E: Get Slave Message Count
 * 
 * HARDWARE REQUIRED:
 * - Arduino MEGA recommended (more memory for logging)
 * - USB or RS485 connection
 * 
 * VALIDATED WORKFLOW:
 * - This example was validated with Modbus on USB Serial0 using `slave.setPort(Serial)`.
 * - Keep Serial Monitor closed while the Modbus master is connected.
 * - Opening the COM port resets the board on many Arduino targets, so wait about
 *   2 to 3 seconds before the first request.
 * 
 * HOW TO CHANGE SERIAL PORTS AND ENABLE DEBUG:
 * - Default: leave Modbus on `Serial` to match the validated USB workflow.
 * - MEGA / multi-UART boards: move Modbus to `Serial1`, `Serial2`, or `Serial3`
 *   if you want to keep USB `Serial` available for debug messages.
 * - Example multi-UART debug setup:
 *     slave.setPort(Serial1);
 *     Serial.begin(115200);
 *     Serial.println(F("Debug output on USB Serial, Modbus on Serial1"));
 * - Single-UART boards should avoid Serial debug while Modbus is active.
 * 
 * MODBUS SLAVE ADDRESS: 1
 * BAUD RATE: 19200
 * 
 * REGISTER MAP:
 * 
 * INPUT REGISTERS (Read with callbacks):
 *    30001: Real-time temperature (generated on-demand)
 *    30002: Uptime seconds (calculated when read)
 *    30003: Free RAM bytes (calculated when read)
 * 
 * HOLDING REGISTERS (Write with callbacks):
 *    40001: Motor Speed Setpoint (triggers speed change)
 *    40002: Alarm Reset Register (triggers alarm clear)
 *    40003: Data Logger Enable (0=off, 1=on)
 * 
 * HOLDING REGISTERS (Diagnostics mirrors):
 *    40010: Bus Message Count (updated continuously)
 *    40011: Communication Error Count
 *    40012: Slave Message Count
 *
 * COILS (Write normalization demo):
 *    00001: Command Coil (callback receives 0x00FF ON / 0x0000 OFF)
 * 
 ******************************************************************************/

/**
 * @example 04_Expert_Callbacks.ino
 * Callback-driven read/write behavior with FC08 diagnostics mirrors.
 */

#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

// Global objects
modbusDevice regBank;
modbusSlave slave;

// State variables
float simulatedTemperature = 25.0;
word motorSpeed = 0;
bool alarmActive = true;
bool dataLoggerEnabled = false;
bool commandCoilActive = false;

// Event log (simple circular buffer)
#define LOG_SIZE 10
struct LogEntry {
  unsigned long timestamp;
  word address;
  word value;
  char type;  // 'R' = read, 'W' = write
};
LogEntry eventLog[LOG_SIZE];
byte logIndex = 0;

unsigned long startTime = 0;
unsigned long lastDiagUpdate = 0;

/*******************************************************************************
 * CALLBACK FUNCTIONS
 ******************************************************************************/

// Called when master reads input register 30001 (Temperature)
word onReadTemperature(word address, modbusDevice *dev) {
  (void)address;  // Unused parameter
  (void)dev;      // Unused parameter
  
  // Generate temperature on-demand (saves having to update it constantly)
  // In real system: read I2C sensor, ADC, etc.
  simulatedTemperature += 0.01;
  if (simulatedTemperature > 30.0) simulatedTemperature = 20.0;
  
  // Log the read event
  logEvent(30001, (word)(simulatedTemperature * 10), 'R');
  
  // Return value (scaled by 10)
  return (word)(simulatedTemperature * 10);
}

// Called when master reads input register 30002 (Uptime)
word onReadUptime(word address, modbusDevice *dev) {
  (void)address;
  (void)dev;
  
  // Calculate uptime in seconds when requested
  unsigned long uptimeSeconds = (millis() - startTime) / 1000;
  
  // Return lower 16 bits (for demo; use setLong for full 32-bit)
  return (word)(uptimeSeconds & 0xFFFF);
}

// Called when master reads input register 30003 (Free RAM)
word onReadFreeRAM(word address, modbusDevice *dev) {
  (void)address;
  (void)dev;

#if defined(__AVR__)
  // AVR boards expose heap internals needed for a simple free-RAM estimate.
  extern int __heap_start, *__brkval;
  int stackTop;
  int freeRam = (int)&stackTop - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);

  return (word)freeRam;
#else
  return 0;
#endif
}

// Called AFTER master writes holding register 40001 (Motor Speed)
void onWriteMotorSpeed(word address, word value, modbusDevice *dev) {
  (void)address;
  
  // Validate speed range
  if (value > 3000) {
    value = 3000;  // Clamp to safe maximum
    dev->set(40001, value);  // Write back clamped value
  }
  
  // Update motor speed (in real system: update PWM, etc.)
  motorSpeed = value;
  
  // Log the write event
  logEvent(40001, value, 'W');
  
  // Mirror to another register for master verification
  if (dev->has(40002)) {
    dev->set(40002, value);
  }
}

// Called AFTER master writes holding register 40002 (Alarm Reset)
void onWriteAlarmReset(word address, word value, modbusDevice *dev) {
  (void)address;
  (void)dev;
  
  // Writing non-zero clears the alarm
  if (value != 0) {
    alarmActive = false;
    logEvent(40002, value, 'W');
    
    // Reset the register to 0 after clearing alarm
    dev->set(address, 0);
  }
}

// Called AFTER master writes holding register 40003 (Data Logger Enable)
void onWriteDataLogger(word address, word value, modbusDevice *dev) {
  (void)address;
  (void)dev;
  
  dataLoggerEnabled = (value != 0);
  logEvent(40003, value, 'W');
}

// Called AFTER master writes coil 00001 (address 1)
void onWriteCommandCoil(word address, word value, modbusDevice *dev) {
  (void)address;
  (void)dev;

  // Core normalization guarantees: ON=0x00FF, OFF=0x0000
  commandCoilActive = (value == 0x00FF);
  logEvent(1, value, 'W');
}

// Called when master sends unsupported function code.
bool onUnknownFunctionReceived(byte functionCode, const byte *request, byte requestLen,
                               byte *response, byte *responseLen, modbusDevice *dev) {
  (void)request;
  (void)requestLen;
  (void)response;
  (void)responseLen;
  (void)dev;

  // Log unknown function attempt and let library fallback send Illegal Function.
  logEvent(0xFFFF, functionCode, 'U');
  return false;
}

/*******************************************************************************
 * HELPER FUNCTIONS
 ******************************************************************************/

// Log events to circular buffer
void logEvent(word address, word value, char type) {
  if (dataLoggerEnabled) {
    eventLog[logIndex].timestamp = millis();
    eventLog[logIndex].address = address;
    eventLog[logIndex].value = value;
    eventLog[logIndex].type = type;
    
    logIndex = (logIndex + 1) % LOG_SIZE;
  }
}

// Update diagnostic registers
void updateDiagnosticRegisters() {
  if (millis() - lastDiagUpdate < 1000) return;
  lastDiagUpdate = millis();
  
  // Read FC08 diagnostic counters from slave
  word busMsgCount = slave.getBusMessageCount();
  word busErrCount = slave.getBusCommunicationErrorCount();
  word slaveMsgCount = slave.getSlaveMessageCount();
  
  // Mirror to holding registers for easy master polling
  regBank.set(40010, busMsgCount);
  regBank.set(40011, busErrCount);
  regBank.set(40012, slaveMsgCount);
}

/*******************************************************************************
 * SETUP
 ******************************************************************************/
void setup() {
  startTime = millis();
  
  // Set slave ID
  regBank.setId(1);
  
  // ===== INPUT REGISTERS (with read callbacks) =====
  regBank.add(30001);  // Temperature (callback-generated)
  regBank.add(30002);  // Uptime (callback-calculated)
  regBank.add(30003);  // Free RAM (callback-calculated)
  
  // Initialize with dummy values (callbacks will override)
  regBank.set(30001, 0);
  regBank.set(30002, 0);
  regBank.set(30003, 0);

  // ===== COILS (write normalization demo) =====
  regBank.add(1);      // Command coil (00001)
  regBank.set(1, 0);   // OFF
  
  // ===== HOLDING REGISTERS (with write callbacks) =====
  regBank.add(40001);  // Motor speed setpoint
  regBank.add(40002);  // Alarm reset
  regBank.add(40003);  // Data logger enable
  
  regBank.set(40001, motorSpeed);
  regBank.set(40002, 0);
  regBank.set(40003, dataLoggerEnabled ? 1 : 0);
  
  // ===== DIAGNOSTIC MIRROR REGISTERS =====
  regBank.add(40010);  // Bus message count
  regBank.add(40011);  // Communication error count
  regBank.add(40012);  // Slave message count
  
  regBank.set(40010, 0);
  regBank.set(40011, 0);
  regBank.set(40012, 0);
  
  // ===== MODBUS CONFIGURATION =====
  slave.setDevice(&regBank);
  // Validated USB Serial0 profile. If you move Modbus to Serial1/2/3,
  // you can safely use Serial.begin()/Serial.print() for debug logging.
  slave.setPort(Serial);
  slave.setProtocol(RTU);  // Default framing mode
  slave.setBaud(19200);
  
  // ===== REGISTER CALLBACKS =====
  // Read callbacks (called BEFORE value is returned to master)
  slave.onRead(30001, onReadTemperature);
  slave.onRead(30002, onReadUptime);
  slave.onRead(30003, onReadFreeRAM);
  
  // Write callbacks (called AFTER value is written to register)
  slave.onWrite(1, onWriteCommandCoil);
  slave.onWrite(40001, onWriteMotorSpeed);
  slave.onWrite(40002, onWriteAlarmReset);
  slave.onWrite(40003, onWriteDataLogger);
  
  // Unknown function callback
  slave.onUnknownFunction(onUnknownFunctionReceived);
  
  // ===== OPTIONAL: CLEAR DIAGNOSTIC COUNTERS =====
  // slave.clearDiagnosticsCounters();
}

/*******************************************************************************
 * MAIN LOOP
 ******************************************************************************/
void loop() {
  // Process Modbus requests
  slave.run();
  
  // Update diagnostic mirror registers periodically
  updateDiagnosticRegisters();
  
  // Simulate alarm condition
  static unsigned long lastAlarmCheck = 0;
  if (millis() - lastAlarmCheck > 5000) {
    lastAlarmCheck = millis();
    
    // Randomly trigger alarm (for demo)
    if (!alarmActive && random(100) < 20) {
      alarmActive = true;
    }
  }
}

/*******************************************************************************
 * TESTING GUIDE
 * 
 * 1. TEST READ CALLBACKS:
 *    - Read input register 30001 (FC04) repeatedly
 *    - Temperature should increment on each read
 *    - This demonstrates on-demand value generation
 * 
 * 2. TEST WRITE CALLBACKS:
 *    - Write holding register 40001 (FC06) with value 1500
 *    - Check that motor speed updated immediately
 *    - Try writing value 5000 (should be clamped to 3000)
 *
 * 2b. TEST COIL NORMALIZATION (FC05/FC15):
 *    - Write coil 00001 ON/OFF
 *    - Callback receives normalized values only:
 *      ON  = 0x00FF
 *      OFF = 0x0000
 * 
 * 3. TEST ALARM RESET:
 *    - Write holding register 40002 (FC06) with value 1
 * 
 * STARTUP NOTE:
 * - If the first request times out right after COM open, wait 2 to 3 seconds and retry.
 *   That behavior is typically caused by Arduino auto-reset, not by callback failure.
 *    - Alarm should clear
 *    - Read register back - should be 0 (auto-reset)
 * 
 * 4. TEST DATA LOGGER:
 *    - Write holding register 40003 with value 1 (enable logging)
 *    - Perform reads/writes
 *    - Event log will capture activity
 * 
 * 5. TEST DIAGNOSTICS (FC08):
 *    Your master tool may support FC08 diagnostics:
 *    - FC08 subfunction 0x000B: returns bus message count
 *    - FC08 subfunction 0x000C: returns error count
 *    - FC08 subfunction 0x000E: returns slave message count
 *    
 *    Or read from mirror registers (easier):
 *    - Read holding register 40010 (FC03): bus message count
 *    - Read holding register 40011 (FC03): communication error count
 *    - Readholding register 40012 (FC03): slave message count
 * 
 * 6. TEST UNKNOWN FUNCTION:
 *    - Send unsupported function code (e.g., FC20)
 *    - Slave should respond with exception 0x01 (Illegal Function)
 *    - Event logged in callback
 * 
 * 7. PERFORMANCE MONITORING:
 *    - Monitor communication error count (40011)
 *    - Should remain 0 for healthy link
 *    - If incrementing: check baud rate, wiring, noise
 *
 * SERIAL PORT NOTES:
 * - This example uses Serial (USB UART0) for Modbus on all boards.
 * - Close Serial Monitor while testing with an external Modbus master.
 * - After opening COM port from a PC master, wait ~2-3s before first request
 *   because Arduino USB serial open typically triggers a board reset.
 * 
 ******************************************************************************/

/*******************************************************************************
 * ADVANCED TOPICS
 * 
 * CALLBACK EXECUTION ORDER:
 * 1. Request received and parsed
 * 2. Address validation
 * 3. onRead() callback (if registered) - modifies return value
 * 4. Value sent to master
 * OR
 * 1. Request received and parsed
 * 2. Address validation
 * 3. Register value updated
 * 4. onWrite() callback (if registered) - reacts to new value
 * 5. Response sent to master
 * 
 * CALLBACK LIMITATIONS:
 * - Maximum 8 read callbacks (MODBUS_MAX_READ_CALLBACKS)
 * - Maximum 8 write callbacks (MODBUS_MAX_WRITE_CALLBACKS)
 * - To increase: edit MODBUS_MAX_READ_CALLBACKS in modbus.h
 * 
 * CALLBACK PERFORMANCE:
 * - Keep callbacks fast (<1ms execution time)
 * - Don't use delay() inside callbacks
 * - Don't perform I2C/SPI transactions in callbacks (use flags instead)
 * 
 * DIAGNOSTIC COUNTER USES:
 * - Bus Message Count: Total frames seen on bus (all slaves)
 * - Bus Error Count: CRC failures, frame errors
 * - Slave Message Count: Messages addressed to this slave
 * - Ratio of (slave/bus) indicates bus loading/congestion
 * 
 ******************************************************************************/
