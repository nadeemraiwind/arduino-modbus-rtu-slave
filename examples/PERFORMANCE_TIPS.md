# Performance Optimization Tips

This document contains micro-level performance tips for industrial deployments.

## Industrial Setup Rules (Required for Reliable USB Serial0 Testing)

These rules came from practical integration findings and should be treated as mandatory when Modbus uses Serial over USB.

1. Auto-reset rule:
- On common Arduino boards (UNO/MEGA class), opening the COM port typically resets the MCU.

2. Startup delay rule:
- Modbus masters should wait about 2.5 seconds after connection before sending the first request.

3. Serial conflict rule:
- Keep Arduino Serial Monitor closed during Modbus traffic on Serial0.
- Avoid Serial.print output while Modbus is using Serial0.

4. Single-UART board rule:
- On single-UART boards, USB Modbus and USB debug logging cannot be used concurrently.

5. Multi-UART board rule:
- On MEGA-class boards, move Modbus to Serial1/Serial2/Serial3 if you need USB Serial debug logs.

## Memory Management

### 1. Use Static Memory Pools for 24/7 Operation

For systems that run continuously without resets, enable static memory pools to avoid heap fragmentation:

```cpp
// Add before including modbus headers:
#define MODBUS_USE_STATIC_REG_POOL 1
#define MODBUS_MAX_DIG_REGS 32    // Digital registers (coils + discrete inputs)
#define MODBUS_MAX_ANA_REGS 64    // Analog registers (input + holding)

#include <modbus.h>
#include <modbusDevice.h>
// ... rest of includes
```

**Memory Calculation:** `(32 * 6) + (64 * 6) = 576 bytes` of static RAM instead of dynamic allocation.

### 2. Minimize Register Count

Only add registers you actually use:

```cpp
// BAD: Adding registers "just in case"
for (word addr = 40001; addr <= 40100; addr++) {
  regBank.add(addr);  // 100 registers = ~600 bytes
}

// GOOD: Add only what you need
regBank.add(40001);  // Temperature
regBank.add(40002);  // Pressure
regBank.add(40003);  // Flow rate
// Only 3 registers = ~18 bytes
```

### 3. Reduce Callback Array Sizes

If you only need a few callbacks, reduce array overhead:

```cpp
// Before includes:
#define MODBUS_MAX_READ_CALLBACKS 3   // Default is 8 (saves 40 bytes)
#define MODBUS_MAX_WRITE_CALLBACKS 3  // Default is 8 (saves 40 bytes)
```

---

## Response Time Optimization

### 1. Use Faster Baud Rates

Higher baud rates reduce request-response latency:

```cpp
// Slow: 9600 baud = ~11ms for 10-byte frame
slave.setBaud(9600);

// Fast: 115200 baud = ~0.9ms for 10-byte frame
slave.setBaud(115200);
```

**Caution:** Higher baud rates are more sensitive to cable noise. Test stability with your actual wiring.

### 2. Optimize Register Lookup

Register access is O(n) by default. Keep frequently-accessed registers at low addresses:

```cpp
// GOOD: Critical registers first (faster lookup)
regBank.add(40001);  // Critical: Emergency stop
regBank.add(40002);  // Critical: Speed setpoint
regBank.add(40050);  // Non-critical: Statistics

// Registers added first are found faster in linked list
```

### 3. Use atomicGet/atomicSet for Thread-Safe ISR Access

When sharing registers between `run()` and interrupt handlers:

```cpp
ISR(TIMER1_COMPA_vect) {
  word value = regBank.atomicGet(40001);  // Interrupt-safe
  // Process value...
  regBank.atomicSet(40002, result);       // Interrupt-safe
}
```

---

## CPU Efficiency

### 1. Avoid Blocking Operations in loop()

```cpp
void loop() {
  slave.run();  // Non-blocking, returns immediately
  
  // BAD: Blocking delays prevent Modbus processing
  // delay(1000);
  
  // GOOD: Non-blocking timing
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    updateSensors();
  }
}
```

### 2. Minimize Callback Computation

Callbacks execute during Modbus request processing. Keep them fast:

```cpp
// BAD: Heavy computation in callback
word onReadTemp(word address, modbusDevice *dev) {
  float temp = readComplexSensor();     // 50ms I2C transaction
  return (word)(temp * 10);
}

// GOOD: Pre-compute in loop(), return cached value
float cachedTemp = 25.0;

void loop() {
  slave.run();
  
  static unsigned long lastRead = 0;
  if (millis() - lastRead > 100) {
    lastRead = millis();
    cachedTemp = readComplexSensor();   // Update cache
  }
}

word onReadTemp(word address, modbusDevice *dev) {
  return (word)(cachedTemp * 10);       // Instant return
}
```

### 3. Disable Unused Serial Ports

On multi-UART boards (MEGA), disable unused serial ports to save CPU cycles:

```cpp
// If using Serial for Modbus:
slave.setPort(Serial);

// Ensure Serial1/2/3 are not initialized
// Don't call Serial1.begin(), etc.
```

---

## Network Performance

### 1. RS485 Timing Tuning

For RS485 networks, tune direction control delays for your transceiver:

```cpp
// Fast transceiver (MAX485):
slave.setTxEnableDelaysUs(10, 10);  // Microsecond precision

// Slow transceiver:
slave.setTxEnableDelays(1, 1);      // Millisecond delays
```

### 2. Monitor Diagnostic Counters

Check for communication issues:

```cpp
void loop() {
  slave.run();
  
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 60000) {  // Every minute
    lastCheck = millis();
    
    word errors = slave.getBusCommunicationErrorCount();
    if (errors > 100) {
      // High error rate: check baud, wiring, termination
      Serial.print("WARNING: ");
      Serial.print(errors);
      Serial.println(" Modbus errors");
    }
  }
}
```

---

## Benchmark Results

Performance measured on Arduino UNO (16 MHz ATmega328P):

| Configuration               | Response Time | RAM Usage | Notes                 |
| --------------------------- | ------------- | --------- | --------------------- |
| 10 registers, 9600 baud     | ~15ms         | 180 bytes | Baseline              |
| 10 registers, 115200 baud   | ~2ms          | 180 bytes | 7.5x faster           |
| 50 registers, dynamic       | ~18ms         | 500 bytes | O(n) lookup penalty   |
| 50 registers, static pool   | ~18ms         | 876 bytes | Predictable memory    |
| With 5 callbacks            | ~15ms         | 260 bytes | Minimal overhead      |

**Test Setup:** FC03 read 10 holding registers from address 40001.

---

## Industrial Best Practices

1. **Always enable watchdog timer** for automatic recovery from lockups
2. **Log diagnostic counters** to EEPROM for failure analysis
3. **Implement fail-safe defaults** for critical control registers
4. **Use atomic transactions** when updating multi-register values
5. **Test at temperature extremes** - timing can vary with clock drift

---

**For More Information:**

- See `ARCHITECTURE.md` for protocol timing details
- See `STATUS.md` for known validation limits and deployment constraints
- See examples `01_Basic` through `05_RS485` for implementation patterns
