// Minimal Modbus RTU Slave Example
// Simple setup with 1 register per function type

#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

// Modbus slave address
#define SLAVE_ID 1

// Create register bank and slave objects
modbusDevice regBank;
modbusSlave slave;

void setup() {
  // Initialize serial communication at hardware level
  Serial.begin(9600);
  
  // Set slave ID
  regBank.setId(SLAVE_ID);
  
  // ========== REGISTER CONFIGURATION ==========
  // Configure register bank (1 register per type)
  // This library uses Modicon addressing (1-based, with type prefixes)
  
  // Coils (Read/Write bits) - FC01 Read, FC05 Write
  // Address range: 1-9999
  regBank.add(1);               // Add coil at address 1
  // To add more coils:
  // regBank.add(2);
  // regBank.add(3);
  // regBank.add(4);
  
  // Discrete Inputs (Read-only bits) - FC02 Read
  // Address range: 10001-19999
  regBank.add(10001);           // Add discrete input at address 10001
  // To add more discrete inputs:
  // regBank.add(10002);
  // regBank.add(10003);
  // regBank.add(10004);
  
  // Input Registers (Read-only 16-bit) - FC04 Read
  // Address range: 30001-39999
  regBank.add(30001);           // Add input register at address 30001
  // To add more input registers:
  // regBank.add(30002);
  // regBank.add(30003);
  // regBank.add(30004);
  
  // Holding Registers (Read/Write 16-bit) - FC03 Read, FC06 Write
  // Address range: 40001-49999
  regBank.add(40001);           // Add holding register at address 40001
  // To add more holding registers:
  // regBank.add(40002);
  // regBank.add(40003);
  // regBank.add(40004);
  
  // ========== SET INITIAL VALUES ==========
  // Set initial values for registers
  regBank.set(1, 0);            // Set coil 1 to OFF (0)
  // regBank.set(1, 1);         // Set coil 1 to ON (1)
  // regBank.set(2, 1);         // Set coil 2 to ON
  
  // Set initial values for discrete inputs
  regBank.set(10001, 0);        // Set discrete input to 0
  
  // Set initial values for input registers
  regBank.set(30001, 250);      // Set input register to 250
  
  // Set initial values for holding registers
  regBank.set(40001, 100);      // Set holding register to 100
  // regBank.set(40002, 250);   // Set holding register 40002 to 250
  
  // ========== MODBUS SLAVE CONFIGURATION ==========
  // Initialize Modbus slave
  slave.setDevice(&regBank);    // Link the register bank to the slave
  slave.setPort(Serial);        // Use Serial port (RX0/TX0 on UNO)
  slave.setBaud(9600);          // Set baud rate to 9600
  
  // Optional: Use different serial port (for Arduino MEGA)
  // For MEGA, you can use Serial1, Serial2, or Serial3:
  // slave.setPort(Serial1);    // Use Serial1 instead of Serial
  
  // Optional: Change baud rate
  // Common baud rates: 9600, 19200, 38400, 57600, 115200
  // slave.setBaud(19200);      // 19200 bps
  // slave.setBaud(38400);      // 38400 bps
  // slave.setBaud(115200);     // 115200 bps
  
  // ========== OPTIONAL RS485 DIRECTION CONTROL ==========
  // Uncomment these lines if using RS485 with DE/RE control pins
  // slave.setTxEnablePin(2, true);      // Pin 2 controls DE/RE, active HIGH
  // slave.setTxEnableDelays(0, 1);      // Pre-delay: 0ms, Post-delay: 1ms
  
  // Parameters explained:
  // setTxEnablePin(pin, activeHigh)
  //   - pin: Arduino pin number connected to DE/RE
  //   - activeHigh: true = HIGH when transmitting, false = LOW when transmitting
  //
  // setTxEnableDelays(preDelayMs, postDelayMs)
  //   - preDelayMs: delay before starting transmission (milliseconds)
  //   - postDelayMs: delay after transmission completes (milliseconds)
  //
  // Typical RS485 modules: MAX485, MAX3485, SN75176
}

void loop() {
  // ========== MAIN MODBUS PROCESSING ==========
  // Process incoming Modbus requests - THIS IS REQUIRED
  slave.run();                  // Check for and process Modbus requests
  
  // ========== READING/WRITING REGISTER VALUES ==========
  
  // Example: Read current value from a register
  // word coilValue = regBank.get(1);       // Get coil 1 value (0 or 1)
  // word hregValue = regBank.get(40001);   // Get holding register value
  
  // Example: Update register values based on sensor readings
  // int sensorValue = analogRead(A0);      // Read analog sensor (0-1023)
  // regBank.set(30001, sensorValue);       // Update input register 30001
  
  // Example: React to holding register writes from master
  // This is how you create PLC-like logic based on master commands
  // if (regBank.get(40001) > 500) {        // Check holding register value
  //   digitalWrite(LED_BUILTIN, HIGH);     // Turn on LED
  // } else {
  //   digitalWrite(LED_BUILTIN, LOW);      // Turn off LED
  // }
  
  // Example: Mirror coil states to discrete inputs
  // regBank.set(10001, regBank.get(1));    // Copy coil 1 to discrete input 10001
  
  // ========== SAFE REGISTER ACCESS ==========
  // IMPORTANT: Always check if a register exists before using it!
  // Using get() on a non-existent register returns MODBUS_REG_NOT_FOUND (0xFFFF).
  // Because 0xFFFF can also be valid data, has() is still the safest approach.
  // Use has() to verify the register was added:
  //
  // if (regBank.has(40005)) {              // Check if register 40005 exists
  //   word value = regBank.get(40005);     // Safe to read
  //   // Use the value...
  // } else {
  //   // Register doesn't exist, handle error
  // }
  
  // ========== SUPPORTED MODBUS FUNCTIONS ==========
  // This library supports these Modbus RTU function codes:
  // FC01 (0x01): Read Coils (addresses 1-9999)
  // FC02 (0x02): Read Discrete Inputs (addresses 10001-19999)
  // FC03 (0x03): Read Holding Registers (addresses 40001-49999)
  // FC04 (0x04): Read Input Registers (addresses 30001-39999)
  // FC05 (0x05): Write Single Coil
  // FC06 (0x06): Write Single Holding Register
  // FC15 (0x0F): Write Multiple Coils
  // FC16 (0x10): Write Multiple Holding Registers
}
