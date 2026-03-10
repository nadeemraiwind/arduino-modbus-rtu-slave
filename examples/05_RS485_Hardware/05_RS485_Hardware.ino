/*******************************************************************************
 * LEVEL 5: RS485 HARDWARE - Industrial Modbus RTU Setup
 * 
 * WHAT YOU'LL LEARN:
 * - RS485 transceiver wiring (MAX485, MAX3485, SN75176, etc.)
 * - DE/RE (Driver Enable/Receiver Enable) pin control
 * - Timing considerations for RS485 direction switching
 * - Microsecond-precision delays for high-speed operation
 * - Multi-drop bus topology (multiple slaves on one bus)
 * - Termination resistors and grounding
 * - Troubleshooting RS485 communication issues
 * 
 * WHY USE RS485?
 * RS485 is the standard for industrial Modbus RTU because:
 * - Long distance: up to 1200m (4000 feet)
 * - Multi-drop: up to 32 devices on one bus (247 with repeaters)
 * - Noise immunity: differential signaling rejects EMI/RFI
 * - Simple wiring: only 3 wires (A, B, GND)
 * - Robust: industrial temperature range, ESD protection
 * 
 * HARDWARE REQUIRED:
 * - Arduino UNO, MEGA, or compatible
 * - RS485 transceiver module (MAX485, MAX3485, MAX13487, etc.)
 * - Twisted pair cable (Cat5e or industrial RS485 cable)
 * - 120Ω termination resistors (for bus endpoints)
 * - 5V power supply (if module requires separate power)
 * 
 * RS485 MODULES:
 * Common modules and pin names:
 * 
 * MAX485 Module:
 *   VCC  - 5V power
 *   GND  - Ground
 *   DI   - Data In (TX from Arduino)
 *   DE   - Driver Enable (HIGH = transmit mode)
 *   RE   - Receiver Enable (LOW = receive mode)
 *   RO   - Receiver Out (RX to Arduino)
 *   A    - RS485 A+ (non-inverted)
 *   B    - RS485 B- (inverted)
 * 
 * Note: DE and RE are usually tied together and controlled by one Arduino pin
 * 
 * WIRING CONNECTIONS:
 * 
 * Arduino UNO/NANO:
 *   Pin 0 (RX) -----> MAX485 RO (Receiver Out)
 *   Pin 1 (TX) -----> MAX485 DI (Data In)
 *   Pin 2      -----> MAX485 DE and RE (tied together)
 *   5V         -----> MAX485 VCC
 *   GND        -----> MAX485 GND
 * 
 * Arduino MEGA:
 *   Pin 19 (RX1) ---> MAX485 RO
 *   Pin 18 (TX1) ---> MAX485 DI
 *   Pin 2        ---> MAX485 DE/RE
 *   5V           ---> MAX485 VCC
 *   GND          ---> MAX485 GND
 * 
 * RS485 Bus (Multiple Slaves):
 *   Slave 1 A --|
 *   Slave 2 A --|---- Master A
 *   Slave 3 A --|
 *   
 *   Slave 1 B --|
 *   Slave 2 B --|---- Master B
 *   Slave 3 B --|
 *   
 *   All GND connected (common ground)
 * 
 * TERMINATION RESISTORS:
 * - 120Ω resistor across A-B at BOTH ends of the bus
 * - First slave and master (endpoints) need termination
 * - Middle slaves do NOT need termination
 * - Without termination: reflections cause bit errors
 * 
 * MODBUS SLAVE ADDRESS: 10 (change as needed for your bus)
 * BAUD RATE: 38400 (common industrial rate)
 * 
 * VALIDATED WORKFLOW VS PRODUCTION RS485:
 * - Development validation confirmed this sketch logic in a USB Serial0 profile.
 * - Real RS485 deployment still requires hardware-specific validation of DE/RE timing,
 *   cable quality, and bus topology.
 * 
 * HOW TO CHANGE SERIAL PORTS AND ENABLE DEBUG:
 * - UNO/NANO production path: Modbus usually stays on `Serial`, so USB debug output is
 *   not safe while traffic is active.
 * - MEGA production path: keep Modbus on `Serial1` and use USB `Serial` for debug prints.
 * - USB-only validation path: force `slave.setPort(Serial)` and keep Serial Monitor closed.
 * - If Modbus uses `Serial`, do not use `Serial.print()` and allow 2 to 3 seconds after
 *   COM open before the first request.
 * 
 ******************************************************************************/

/**
 * @example 05_RS485_Hardware.ino
 * RS485 direction control and deployment-oriented timing guidance.
 */

#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

// ===== RS485 CONFIGURATION =====
#define RS485_DE_PIN 2          // Driver Enable / Receiver Enable pin
#define RS485_DE_ACTIVE HIGH    // Some modules use HIGH, others use LOW

// Timing delays (microseconds) for RS485 direction switching
// Adjust these if you see communication errors at high speeds
#define RS485_PRE_DELAY_US  100   // Delay before transmitting (us)
#define RS485_POST_DELAY_US 100   // Delay after transmitting (us)

// Practical tuning for slower transceivers / long cables:
// charTimeUs ~= 10000000 / baud  for 8N1 (10 bits/char)
// Example at 38400 baud: charTimeUs ~= 260us
// MAX485-class transceivers commonly need ~10-50us of DE/RE settling time
// before/after TX for reliable first/last-bit integrity.
// If pre-delay is too short, the first response bit is clipped and masters
// usually report CRC/frame errors.
// A common starting point is:
//   preDelay  = 0.25 to 0.5 char times
//   postDelay = 1.0 to 2.0 char times
// For MAX485 on noisy or long links, increase post delay first.

// ===== MODBUS CONFIGURATION =====
#define SLAVE_ADDRESS 10        // Set unique address for each slave (1-247)
#define BAUD_RATE     38400     // Common rates: 9600, 19200, 38400, 115200

// Global objects
modbusDevice regBank;
modbusSlave slave;

// Demo process variables
word temperature = 250;   // 25.0°C × 10
word pressure = 120;      // 12.0 bar × 10
word flowRate = 100;      // 10.0 L/min × 10

unsigned long lastUpdate = 0;

void setup() {
  // ===== SLAVE CONFIGURATION =====
  regBank.setId(SLAVE_ADDRESS);
  
  // Add registers for demonstration
  regBank.add(30001);  // Input register: Temperature
  regBank.add(30002);  // Input register: Pressure
  regBank.add(30003);  // Input register: Flow rate
  regBank.add(40001);  // Holding register: Setpoint 1
  regBank.add(40002);  // Holding register: Setpoint 2
  
  // Set initial values
  regBank.set(30001, temperature);
  regBank.set(30002, pressure);
  regBank.set(30003, flowRate);
  regBank.set(40001, 1000);
  regBank.set(40002, 500);
  
  // ===== MODBUS SERIAL CONFIGURATION =====
  // Production default:
  // - UNO/NANO: Modbus on Serial (pins 0/1), so USB debug is unavailable during traffic.
  // - MEGA: Modbus on Serial1 so USB Serial can remain available for logging.
  // Validation alternative:
  // - Force slave.setPort(Serial) to match the USB Serial0 test profile.
  
  slave.setDevice(&regBank);
  
  #if defined(__AVR_ATmega2560__)  // Arduino MEGA
    slave.setPort(Serial1);  // Production default: Modbus on Serial1, USB Serial free for debug.
  #else  // Arduino UNO, NANO, etc.
    slave.setPort(Serial);   // Single-UART boards use Serial for Modbus; avoid Serial debug output.
  #endif
  
  slave.setProtocol(RTU);      // Default framing mode (see Level 7 for ASCII)
  slave.setBaud(BAUD_RATE);
  
  // ===== RS485 DIRECTION CONTROL =====
  // CRITICAL: Enable this in real RS485 deployments so DE/RE follows TX state.
  slave.setTxEnablePin(RS485_DE_PIN, RS485_DE_ACTIVE);
  
  // ===== RS485 TIMING DELAYS =====
  // Use microsecond-precision delays for high-speed RS485.
  // This path is documented and compiled here, but final values must be tuned on
  // your real bus because USB Serial0 testing does not validate DE/RE timing.
  // Gold-standard demo profile for microsecond RS485 control.
  slave.setTxEnableDelaysUs(10, 10);
  
  // Millisecond version (less precise, but simpler):
  // slave.setTxEnableDelays(0, 1);  // 0ms pre, 1ms post
  
  // ===== TIMING GUIDELINES BY BAUD RATE =====
  // 9600 baud:   1-2ms post-delay is safe
  // 19200 baud:  0.5-1ms post-delay
  // 38400 baud:  100-200us post-delay (use setTxEnableDelaysUs)
  // 115200 baud: 50-100us post-delay (use setTxEnableDelaysUs)
  //
  // Rule of thumb: post-delay ≥ time for 1 character at baud rate
  // At 9600 baud: 1 char ≈ 1.04ms
  // At 38400 baud: 1 char ≈ 260us
  // At 115200 baud: 1 char ≈ 87us
  // If response truncation appears, increase RS485_POST_DELAY_US.
  // If first bytes are lost, increase RS485_PRE_DELAY_US.
}

void loop() {
  // Process Modbus requests
  slave.run();
  
  // Update process values every second
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    
    // Simulate changing sensor values
    temperature++;
    if (temperature > 350) temperature = 200;
    
    pressure++;
    if (pressure > 150) pressure = 100;
    
    flowRate += 5;
    if (flowRate > 200) flowRate = 50;
    
    // Update Modbus registers
    regBank.set(30001, temperature);
    regBank.set(30002, pressure);
    regBank.set(30003, flowRate);
  }
}

/*******************************************************************************
 * WIRING CHECKLIST
 * 
 * ✓ VCC connected to 5V
 * ✓ GND connected to Arduino GND
 * ✓ DI connected to TX pin (1 on UNO, 18 on MEGA)
 * ✓ RO connected to RX pin (0 on UNO, 19 on MEGA)
 * ✓ DE and RE tied together, connected to control pin (2)
 * ✓ A and B connected to twisted pair cable
 * ✓ 120Ω termination resistor across A-B (if endpoint)
 * ✓ Common ground between all devices on bus
 * 
 ******************************************************************************/

/*******************************************************************************
 * TESTING PROCEDURE
 * 
 * 1. INITIAL POWER-UP TEST:
 *    - Upload this sketch to Arduino
 *    - Disconnect USB (or keep connected if using MEGA with separate Serial)
 *    - Connect RS485 module
 *    - Power Arduino from external 5-12V supply
 * 
 * 2. SINGLE SLAVE TEST:
 *    - Connect ONE slave to master
 *    - Master should read registers successfully
 *    - If no response: check wiring, baud rate, slave address
 * 
 * 3. MULTI-SLAVE TEST:
 *    - Upload this sketch to multiple Arduinos
 *    - Change SLAVE_ADDRESS to unique values (10, 11, 12, etc.)
 *    - Connect all slaves to same A-B bus
 *    - Master can now poll different slave addresses
 * 
 * 4. BAUD RATE TEST:
 *    - Start at 9600 (most reliable)
 *    - Test at 19200 (good balance)
 *    - Try 38400 (requires good wiring)
 *    - Test 115200 (requires excellent wiring and short distances)
 * 
 * 5. DISTANCE TEST:
 *    - Start with 1-2 meter cable
 *    - Gradually increase distance
 *    - Watch for CRC errors or timeouts
 *    - Maximum distance: ~1200m at 9600 baud with good cable
 * 
 ******************************************************************************/

/*******************************************************************************
 * TROUBLESHOOTING GUIDE
 * 
 * PROBLEM: No response from slave
 * SOLUTION:
 *   - Check slave address matches master's target
 *   - Verify baud rate, parity, stop bits match (8N1)
 *   - Confirm A-B wiring (not swapped)
 *   - Measure voltage on A and B (should be ~2.5V differential)
 *   - Check DE/RE pin control (should toggle during transmission)
 * 
 * PROBLEM: Garbled data or CRC errors
 * SOLUTION:
 *   - Add/check termination resistors (120Ω at endpoints)
 *   - Use shielded twisted pair cable
 *   - Reduce baud rate (try 9600 instead of 38400)
 *   - Shorten cable distance
 *   - Check for loose connections
 *   - Remove noise sources (motors, relays nearby)
 * 
 * PROBLEM: Works with one slave, fails with multiple
 * SOLUTION:
 *   - Verify each slave has unique address
 *   - Check common ground between all devices
 *   - Ensure only endpoint devices have termination resistors
 *   - Use star topology (NOT daisy-chain if possible)
 *   - Increase slave response timeout in master
 * 
 * PROBLEM: Slave receives but can't transmit
 * SOLUTION:
 *   - Check DE/RE pin wiring (pin 2 in this example)
 *   - Verify setTxEnablePin() pin number matches wiring
 *   - Try inverting active level (HIGH vs LOW)
 *   - Increase post-delay: slave.setTxEnableDelaysUs(0, 500);
 *   - Some modules have LED indicators - watch for TX activity
 * 
 * PROBLEM: Works for a while, then stops
 * SOLUTION:
 *   - Check power supply voltage and current capacity
 *   - Module may be overheating (add heatsink if needed)
 *   - EMI/RFI interference (add ferrite beads on cable)
 *   - Ground loops (isolate grounds or use isolated RS485 modules)
 * 
 * PROBLEM: Works at 9600, fails at 38400
 * SOLUTION:
 *   - Cable quality insufficient for high speed
 *   - Cable too long (reduce distance)
 *   - Termination resistors missing or wrong value
 *   - Use setTxEnableDelaysUs() instead of setTxEnableDelays()
 *   - Check for capacitance in cable (twisted pair is better)
 * 
 ******************************************************************************/

/*******************************************************************************
 * BEST PRACTICES
 * 
 * 1. WIRING:
 *    - Use twisted pair cable (preferably shielded)
 *    - Keep cable runs short for high baud rates
 *    - Avoid running RS485 cable near AC power lines
 *    - Use proper connectors (screw terminals, not breadboard jumpers)
 * 
 * 2. GROUNDING:
 *    - Connect GND between all devices on bus
 *    - Use single-point ground (star topology)
 *    - Shield should be grounded at ONE end only (avoid ground loops)
 * 
 * 3. TERMINATION:
 *    - 120Ω resistor across A-B at FIRST device (usually master)
 *    - 120Ω resistor across A-B at LAST device
 *    - Do NOT terminate middle devices
 *    - Parallel termination: 120Ω || 120Ω = 60Ω bus impedance
 * 
 * 4. BAUD RATE SELECTION:
 *    - 9600: Best reliability, long distances, noisy environments
 *    - 19200: Good balance for most applications
 *    - 38400: Industrial standard, requires quality cable
 *    - 115200: Short distances only (<50m), perfect wiring required
 * 
 * 5. SLAVE ADDRESSING:
 *    - Use sequential addresses: 1, 2, 3, 4...
 *    - Reserve low addresses (1-10) for critical devices
 *    - Document address assignments
 *    - Use DIP switches for easy address configuration
 * 
 * 6. TIMING:
 *    - Keep processing in loop() fast (<10ms ideal)
 *    - Avoid delay() in loop() (breaks Modbus timing)
 *    - Use millis() for non-blocking timing
 *    - slave.run() must be called frequently
 * 
 * 7. DIAGNOSTICS:
 *    - Monitor communication error count (FC08)
 *    - Log failed requests and timeouts
 *    - Use oscilloscope to verify signal integrity
 *    - Test with known-good master device first
 * 
 ******************************************************************************/

/*******************************************************************************
 * HARDWARE MODULES COMPARISON
 * 
 * MAX485 (Most Common):
 *   - Low cost (~$1)
 *   - 2.5 Mbps max
 *   - 32 devices on bus
 *   - No isolation
 *   - Good for short distances (<300m)
 * 
 * MAX3485:
 *   - Improved ESD protection
 *   - 10 Mbps max
 *   - Low power
 *   - Same pinout as MAX485
 * 
 * MAX13487:
 *   - 16 kV ESD protection
 *   - Low EMI
 *   - Hot-swap capability
 *   - Better for harsh industrial environments
 * 
 * SN75176 (Texas Instruments):
 *   - Industrial standard
 *   - Robust and reliable
 *   - Similar to MAX485 pinout
 * 
 * Isolated RS485 Modules:
 *   - Galvanic isolation (usually 2.5 kV)
 *   - Protection from ground loops
 *   - Higher cost (~$10-20)
 *   - Recommended for noisy industrial environments
 *   - Separate power supply sometimes required
 * 
 ******************************************************************************/
