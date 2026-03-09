/*******************************************************************************
 * LEVEL 2: INTERMEDIATE - Multi-Register Process Simulation
 * 
 * WHAT YOU'LL LEARN:
 * - Working with multiple registers of each type
 * - Simulating a real industrial process (temperature, pressure, flow)
 * - Reading/writing values dynamically
 * - Mirroring coil commands to discrete inputs (common PLC pattern)
 * - Safe register access using has() method
 * 
 * SIMULATED PROCESS:
 * This example simulates a small industrial system with:
 * - 5 control coils (pumps, valves, motors)
 * - 5 status indicators (discrete inputs showing device states)
 * - 5 process variables (temperature, pressure, flow, humidity, voltage)
 * - 5 setpoints/parameters (speed, temperature target, valve position, etc.)
 * 
 * HARDWARE REQUIRED:
 * - Arduino UNO, MEGA, or compatible
 * - USB connection for testing
 * 
 * MODBUS SLAVE ADDRESS: 1
 * BAUD RATE: 19200 (faster than basic example)
 * 
 * REGISTER MAP:
 * 
 * COILS (FC01/FC05) - Control Commands from Master:
 *    1: Pump 1 Enable
 *    2: Pump 2 Enable
 *    3: Valve Open
 *    4: Heater Enable
 *    5: Alarm Reset
 * 
 * DISCRETE INPUTS (FC02) - Status Feedback to Master:
 *    10001: Pump 1 Running
 *    10002: Pump 2 Running
 *    10003: Valve Opened
 *    10004: Heater Active
 *    10005: Alarm Cleared
 * 
 * INPUT REGISTERS (FC04) - Process Values (Read-Only):
 *    30001: Temperature (°C × 10)      Example: 245 = 24.5°C
 *    30002: Pressure (bar × 10)        Example: 125 = 12.5 bar
 *    30003: Flow Rate (L/min × 10)     Example: 150 = 15.0 L/min
 *    30004: Humidity (% × 10)          Example: 652 = 65.2%
 *    30005: Voltage (V × 10)           Example: 240 = 24.0V
 * 
 * HOLDING REGISTERS (FC03/FC06) - Setpoints/Parameters (Read/Write):
 *    40001: Speed Setpoint (RPM)
 *    40002: Temperature Setpoint (°C × 10)
 *    40003: Valve Position (% × 10)
 *    40004: Alarm Delay (milliseconds)
 *    40005: PID Gain (× 100)
 * 
 ******************************************************************************/

#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

// Global objects
modbusDevice regBank;
modbusSlave slave;

// Simulated process variables
float temperature = 24.5;      // °C
float pressure = 12.5;         // bar
float flowRate = 15.0;         // L/min
float humidity = 65.2;         // %
float voltage = 24.0;          // V

unsigned long lastProcessUpdate = 0;

void setup() {
  // Set slave ID
  regBank.setId(1);
  
  // ===== REGISTER ALLOCATION =====
  // Add all coils (1-5)
  for (word addr = 1; addr <= 5; addr++) {
    regBank.add(addr);
    regBank.set(addr, 0);  // All OFF initially
  }
  
  // Add all discrete inputs (10001-10005)
  for (word addr = 10001; addr <= 10005; addr++) {
    regBank.add(addr);
    regBank.set(addr, 0);  // All inactive initially
  }
  
  // Add all input registers (30001-30005)
  for (word addr = 30001; addr <= 30005; addr++) {
    regBank.add(addr);
  }
  
  // Add all holding registers (40001-40005)
  for (word addr = 40001; addr <= 40005; addr++) {
    regBank.add(addr);
  }
  
  // ===== INITIAL VALUES =====
  // Process values (scaled by 10 for decimal precision)
  regBank.set(30001, (word)(temperature * 10));  // 245 = 24.5°C
  regBank.set(30002, (word)(pressure * 10));     // 125 = 12.5 bar
  regBank.set(30003, (word)(flowRate * 10));     // 150 = 15.0 L/min
  regBank.set(30004, (word)(humidity * 10));     // 652 = 65.2%
  regBank.set(30005, (word)(voltage * 10));      // 240 = 24.0V
  
  // Setpoints/parameters
  regBank.set(40001, 1500);    // Speed setpoint: 1500 RPM
  regBank.set(40002, 250);     // Temperature setpoint: 25.0°C
  regBank.set(40003, 500);     // Valve position: 50.0%
  regBank.set(40004, 5000);    // Alarm delay: 5000 ms
  regBank.set(40005, 120);     // PID gain: 1.20
  
  // ===== MODBUS CONFIGURATION =====
  slave.setDevice(&regBank);
  slave.setPort(Serial);       // Use Serial (or Serial1 for MEGA)
  slave.setProtocol(RTU);      // Default framing mode
  slave.setBaud(19200);        // Higher baud rate for faster communication
  
  // Optional: RS485 configuration (uncomment if using RS485)
  // slave.setTxEnablePin(2, true);      // Pin 2 for DE/RE, active HIGH
  // slave.setTxEnableDelays(0, 1);      // 0ms pre-delay, 1ms post-delay
}

void loop() {
  // REQUIRED: Process Modbus requests
  slave.run();
  
  // Update process simulation every 500ms
  updateProcessSimulation();
  
  // Mirror coil commands to discrete inputs (common in PLC systems)
  mirrorCoilsToInputs();
  
  // React to master commands
  handleMasterCommands();
}

/*******************************************************************************
 * Process Simulation - Updates sensor readings
 ******************************************************************************/
void updateProcessSimulation() {
  if (millis() - lastProcessUpdate < 500) {
    return;  // Only update every 500ms
  }
  lastProcessUpdate = millis();
  
  // Simulate slowly changing process values
  temperature += 0.1;
  if (temperature > 30.0) temperature = 20.0;
  
  pressure += 0.2;
  if (pressure > 15.0) pressure = 10.0;
  
  flowRate += 0.5;
  if (flowRate > 25.0) flowRate = 10.0;
  
  humidity -= 0.3;
  if (humidity < 50.0) humidity = 70.0;
  
  voltage += 0.05;
  if (voltage > 26.0) voltage = 22.0;
  
  // Write simulated values to input registers (scaled by 10)
  regBank.set(30001, (word)(temperature * 10));
  regBank.set(30002, (word)(pressure * 10));
  regBank.set(30003, (word)(flowRate * 10));
  regBank.set(30004, (word)(humidity * 10));
  regBank.set(30005, (word)(voltage * 10));
}

/*******************************************************************************
 * Mirror Coil Commands to Discrete Inputs
 * 
 * This is a common pattern in industrial systems:
 * - Master writes coils to issue commands (FC05)
 * - Slave mirrors the command state to discrete inputs (FC02)
 * - Master can verify commands were received by reading discrete inputs
 * 
 * In a real system, you would check actual hardware state (pump running,
 * valve position, etc.) before setting the discrete input.
 ******************************************************************************/
void mirrorCoilsToInputs() {
  // SAFE ACCESS PATTERN: Always check if register exists
  if (regBank.has(1) && regBank.has(10001)) {
    regBank.set(10001, regBank.get(1));  // Pump 1 state
  }
  
  if (regBank.has(2) && regBank.has(10002)) {
    regBank.set(10002, regBank.get(2));  // Pump 2 state
  }
  
  if (regBank.has(3) && regBank.has(10003)) {
    regBank.set(10003, regBank.get(3));  // Valve state
  }
  
  if (regBank.has(4) && regBank.has(10004)) {
    regBank.set(10004, regBank.get(4));  // Heater state
  }
  
  if (regBank.has(5) && regBank.has(10005)) {
    regBank.set(10005, regBank.get(5));  // Alarm state
  }
}

/*******************************************************************************
 * Handle Master Commands
 * 
 * Read holding registers written by master and react accordingly.
 * This demonstrates how to create PLC-like logic based on Modbus commands.
 ******************************************************************************/
void handleMasterCommands() {
  // Example: Check speed setpoint and adjust behavior
  if (regBank.has(40001)) {
    word speedSetpoint = regBank.get(40001);
    
    // If master sets speed > 2000 RPM, limit it for safety
    if (speedSetpoint > 2000) {
      regBank.set(40001, 2000);  // Clamp to maximum safe speed
    }
  }
  
  // Example: Check temperature setpoint
  if (regBank.has(40002)) {
    word tempSetpoint = regBank.get(40002);  // Value × 10
    
    // If setpoint is below 15.0°C (150), issue warning via discrete input
    // (In real system, you might set an "alarm" discrete input)
    if (tempSetpoint < 150) {
      // Could set a warning discrete input here
    }
  }
  
  // Example: Monitor valve position changes
  static word lastValvePosition = 500;
  if (regBank.has(40003)) {
    word currentValvePosition = regBank.get(40003);
    
    if (currentValvePosition != lastValvePosition) {
      // Master changed valve position - react here
      // In real system: send PWM signal to servo, log event, etc.
      lastValvePosition = currentValvePosition;
    }
  }
}

/*******************************************************************************
 * TESTING GUIDE
 * 
 * 1. BASIC CONNECTIVITY TEST:
 *    - Read input register 30001 (FC04)
 *    - Watch the temperature value increase over time
 * 
 * 2. CONTROL TEST:
 *    - Write coil 1 to ON (FC05, address 1, value 1)
 *    - Read discrete input 10001 (FC02)
 *    - Verify it mirrors the coil state
 * 
 * 3. SETPOINT TEST:
 *    - Write holding register 40001 (FC06, address 40001, value 1750)
 *    - Read it back (FC03) to verify
 * 
 * 4. MULTI-WRITE TEST:
 *    - Use FC16 to write multiple holding registers at once
 *    - Write addresses 40001-40005 with new values
 * 
 * 5. PERFORMANCE TEST:
 *    - Continuously poll all input registers (30001-30005)
 *    - Monitor update rate and verify no communication errors
 * 
 ******************************************************************************/
