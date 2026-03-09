/*******************************************************************************
 * LEVEL 1: BASIC - Minimal Modbus RTU Slave
 * 
 * WHAT YOU'LL LEARN:
 * - Absolute minimum code to create a working Modbus slave
 * - One register of each type (coil, discrete input, input register, holding register)
 * - How to initialize and run the slave
 * 
 * HARDWARE REQUIRED:
 * - Arduino UNO, MEGA, or compatible board
 * - USB cable for Serial communication
 * - (Optional) RS485 module for industrial use
 * 
 * WIRING:
 * - Connect Arduino USB to PC
 * - Use a Modbus master tool (Modbus Poll, QModMaster, pymodbus, etc.)
 * - Serial settings: 9600 baud, 8 data bits, 1 stop bit, no parity (8N1)
 * 
 * MODBUS SLAVE ADDRESS: 1
 * 
 * TEST WITH MODBUS MASTER:
 * - FC01: Read coil at address 1
 * - FC02: Read discrete input at address 10001
 * - FC03: Read holding register at address 40001
 * - FC04: Read input register at address 30001
 * - FC05: Write coil at address 1 (set to 0 or 1)
 * - FC06: Write holding register at address 40001 (set to any 16-bit value)
 * 
 ******************************************************************************/

#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

// Create two main objects: register bank and slave handler
modbusDevice regBank;    // Stores all registers (coils, inputs, holding regs)
modbusSlave slave;       // Handles Modbus protocol communication

void setup() {
  // STEP 1: Set slave address (1-247)
  regBank.setId(1);
  
  // STEP 2: Add registers (Modicon addressing)
  // Coil (read/write bit): address 1-9999
  regBank.add(1);
  
  // Discrete Input (read-only bit): address 10001-19999
  regBank.add(10001);
  
  // Input Register (read-only 16-bit): address 30001-39999
  regBank.add(30001);
  
  // Holding Register (read/write 16-bit): address 40001-49999
  regBank.add(40001);
  
  // STEP 3: Set initial values
  regBank.set(1, 0);          // Coil OFF
  regBank.set(10001, 0);      // Discrete input OFF
  regBank.set(30001, 100);    // Input register = 100
  regBank.set(40001, 500);    // Holding register = 500
  
  // STEP 4: Configure Modbus slave
  slave.setDevice(&regBank);  // Link register bank to slave
  slave.setPort(Serial);      // Use Serial port (change to Serial1/2/3 for MEGA)
  slave.setProtocol(RTU);     // Default framing mode
  // slave.setProtocol(ASCII); // Optional: use Modbus ASCII framing (:...CRLF)
  slave.setBaud(9600);        // Set baud rate
}

void loop() {
  // STEP 5: Process Modbus requests (THIS IS REQUIRED!)
  slave.run();
  
  // Your application code goes here:
  // - Read local sensors and update input registers
  // - Check holding registers for commands from master
  // - Update coils/discrete inputs based on hardware state
  
  // Example: Slowly increment input register
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    word value = regBank.get(30001);
    regBank.set(30001, value + 1);
  }
}

/*******************************************************************************
 * QUICK REFERENCE - Modbus Address Ranges
 * 
 * Coils (FC01/FC05):              1 -  9999   (read/write bits)
 * Discrete Inputs (FC02):     10001 - 19999   (read-only bits)
 * Input Registers (FC04):     30001 - 39999   (read-only 16-bit words)
 * Holding Registers (FC03/FC06): 40001 - 49999   (read/write 16-bit words)
 * 
 * SUPPORTED FUNCTION CODES:
 * FC01 (0x01) - Read Coils
 * FC02 (0x02) - Read Discrete Inputs
 * FC03 (0x03) - Read Holding Registers
 * FC04 (0x04) - Read Input Registers
 * FC05 (0x05) - Write Single Coil
 * FC06 (0x06) - Write Single Holding Register
 * FC15 (0x0F) - Write Multiple Coils
 * FC16 (0x10) - Write Multiple Holding Registers
 * FC08 (0x08) - Diagnostics (subset)
 * 
 ******************************************************************************/
