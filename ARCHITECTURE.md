# System Architecture and Code Flow {#architecture}

This document provides detailed sequence diagrams and flowcharts showing the internal logic of the Modbus RTU Slave library. Understanding these flows is essential for industrial deployment and debugging.

---

## Request-Response Sequence Diagram {#sequence_diagram}

The following diagram shows the timing and ownership of a complete Modbus transaction, from the moment the master sends a request until the slave responds.

@dot
digraph Sequence {
  rankdir=TB;
  node [shape=box, style=rounded, fontname=Helvetica, fontsize=9];
  edge [fontname=Helvetica, fontsize=8];
  
  // Timeline nodes
  T0 [label="t=0: Master sends FC03 request", fillcolor="#f0f0f0", style=filled];
  T1 [label="t=1ms: Bytes arrive at Serial buffer", fillcolor="#e6f2ff", style=filled];
  T2 [label="t=2ms: run() detects bytes\nState: IDLE → RECEIVING", fillcolor="#e6f2ff", style=filled];
  T3 [label="t=3ms: Last byte received\nSilence timer resets", fillcolor="#e6f2ff", style=filled];
  T4 [label="t=3+3.5char: Silence expires\nState: RECEIVING → COMPLETE", fillcolor="#ffffcc", style=filled];
  T5 [label="t=4ms: processFrame() entry\nValidation chain begins", fillcolor="#ffccff", style=filled];
  T6 [label="Check 1: Length ≥ 8? ✓\nCheck 2: CRC valid? ✓\nCheck 3: Slave ID match? ✓", fillcolor="#ffccff", style=filled];
  T7 [label="Check 4: Function code? ✓\nCheck 5: Address range? ✓", fillcolor="#ffccff", style=filled];
  T8 [label="onRead() callback fires\n[User Application Code]", fillcolor="#ffe6cc", style=filled];
  T9 [label="Register Bank: get(address)\nReturns value(s)", fillcolor="#e6ffe6", style=filled];
  T10 [label="Build response frame\nCalculate CRC", fillcolor="#ffccff", style=filled];
  T11 [label="TX Enable (RS485 DE pin HIGH)\nSend response bytes", fillcolor="#e6f2ff", style=filled];
  T12 [label="TX Disable (RS485 DE pin LOW)\nState: COMPLETE → IDLE", fillcolor="#e6f2ff", style=filled];
  T13 [label="t=6ms: Master receives response\nTransaction complete", fillcolor="#f0f0f0", style=filled];
  
  // Flow
  T0 -> T1 [label="Physical Layer"];
  T1 -> T2 [label="slave.run()"];
  T2 -> T3 [label="Byte collection"];
  T3 -> T4 [label="t3.5 silence"];
  T4 -> T5 [label="Frame complete"];
  T5 -> T6 [label="Security checks"];
  T6 -> T7;
  T7 -> T8 [label="Callback window", color=orange];
  T8 -> T9 [label="Data access"];
  T9 -> T10 [label="Response build"];
  T10 -> T11 [label="Transmit"];
  T11 -> T12;
  T12 -> T13 [label="Physical Layer"];
}
@enddot

**Key Timing Points:**

1. **Byte Reception:** Each incoming byte resets the silence timer
2. **3.5 Character Silence (t3.5):** Marks end of frame based on baud rate (e.g., ~1.8ms at 19200 baud)
3. **Validation Chain:** Sequential checks execute in <500µs
4. **Callback Window:** User code executes between validation and response (orange path)
5. **TX Control:** RS485 DE/RE pin managed automatically with optional pre/post delays
6. **Non-Blocking:** `run()` returns immediately after each state transition (~5-15µs per call)

**Ownership Model:**

| Phase | Owner | Blocking? |
| ----- | ----- | --------- |
| Byte reception | Hardware Serial | No (buffered) |
| State machine | Library (`run()`) | No (returns quickly) |
| Validation chain | Library (`processFrame()`) | No (deterministic) |
| Callback execution | User Application | No (user controls duration) |
| Response transmission | Hardware Serial | No (buffered) |

---

## run() State Machine Flowchart {#state_machine}

The `slave.run()` method implements a non-blocking state machine that never stalls the main loop. This design is critical for industrial applications where the Arduino must perform other tasks.

@dot
digraph StateMachine {
  rankdir=TB;
  node [shape=box, style=rounded, fontname=Helvetica, fontsize=10];
  edge [fontname=Helvetica, fontsize=9];
  
  Start [label="run() Entry", shape=ellipse, fillcolor=lightgreen, style=filled];
  
  CheckAtomic [label="Is atomic lock active?", shape=diamond, fillcolor=lightyellow, style=filled];
  DeferProcessing [label="Defer processing\n(return immediately)", fillcolor=lightcoral, style=filled];
  
  CheckState [label="Switch on\n_state", shape=diamond, fillcolor=lightblue, style=filled];
  
  // IDLE State
  IdleState [label="State: IDLE\nWaiting for first byte", fillcolor="#e6f2ff", style=filled];
  CheckAvailable [label="Serial.available()?", shape=diamond];
  ReadByte [label="Read first byte\nReset buffer\n\_pos = 0", fillcolor="#ccffcc", style=filled];
  TransitionReceiving [label="State = RECEIVING\nReset silence timer", fillcolor="#ffffcc", style=filled];
  
  // RECEIVING State
  ReceivingState [label="State: RECEIVING\nCollecting bytes", fillcolor="#e6f2ff", style=filled];
  CheckBytes [label="Bytes available?", shape=diamond];
  CheckOverflow [label="Buffer full?\n(\_pos >= 256)", shape=diamond];
  SetOverflow [label="Set \_rxOverflow flag\nIncrement error counter", fillcolor=lightcoral, style=filled];
  ReadMoreBytes [label="Read byte(s)\nReset silence timer\nfor each byte", fillcolor="#ccffcc", style=filled];
  CheckSilence [label="Silence > t3.5?", shape=diamond];
  TransitionComplete [label="State = COMPLETE", fillcolor="#ffffcc", style=filled];
  
  // COMPLETE State
  CompleteState [label="State: COMPLETE\nFrame ready", fillcolor="#e6f2ff", style=filled];
  CallProcess [label="Call processFrame()\nValidation + Response", fillcolor="#ffccff", style=filled];
  TransitionIdle [label="State = IDLE\nReset buffer", fillcolor="#ffffcc", style=filled];
  
  Return [label="return\n(Non-blocking)", shape=ellipse, fillcolor=lightgreen, style=filled];
  
  // Flow
  Start -> CheckAtomic;
  CheckAtomic -> DeferProcessing [label="Yes\n(atomic transaction\nin progress)"];
  CheckAtomic -> CheckState [label="No"];
  DeferProcessing -> Return;
  
  CheckState -> IdleState [label="IDLE"];
  CheckState -> ReceivingState [label="RECEIVING"];
  CheckState -> CompleteState [label="COMPLETE"];
  
  // IDLE flow
  IdleState -> CheckAvailable;
  CheckAvailable -> Return [label="No"];
  CheckAvailable -> ReadByte [label="Yes"];
  ReadByte -> TransitionReceiving;
  TransitionReceiving -> Return;
  
  // RECEIVING flow
  ReceivingState -> CheckBytes;
  CheckBytes -> CheckSilence [label="No new bytes"];
  CheckBytes -> CheckOverflow [label="Yes"];
  CheckOverflow -> SetOverflow [label="Yes"];
  CheckOverflow -> ReadMoreBytes [label="No"];
  SetOverflow -> TransitionIdle;
  ReadMoreBytes -> Return;
  CheckSilence -> Return [label="No (< t3.5)"];
  CheckSilence -> TransitionComplete [label="Yes (≥ t3.5)"];
  TransitionComplete -> Return;
  
  // COMPLETE flow
  CompleteState -> CallProcess;
  CallProcess -> TransitionIdle;
  TransitionIdle -> Return;
}
@enddot

**State Transition Rules:**

- **IDLE → RECEIVING:** First byte detected
- **RECEIVING → RECEIVING:** More bytes arriving (silence timer resets)
- **RECEIVING → IDLE:** Buffer overflow detected
- **RECEIVING → COMPLETE:** Silence timer expires (3.5 char delay)
- **COMPLETE → IDLE:** Frame processed and response sent

**Critical Features:**

1. **Non-Blocking Design:** Every path through `run()` returns quickly
2. **Silence Detection:** Uses `micros()` for precise RTU frame timing
3. **Overflow Protection:** Detects and recovers from oversized frames
4. **Atomic Lock Integration:** Defers frame processing during critical sections
5. **Error Tracking:** Increments counters for diagnostics

---

## processFrame() Validation Chain {#validation_chain}

The `processFrame()` method implements a rigorous validation chain before executing any Modbus command. This security-first design prevents malformed frames from corrupting the register bank.

@dot
digraph ValidationChain {
  rankdir=TB;
  node [shape=box, style=rounded, fontname=Helvetica, fontsize=10];
  edge [fontname=Helvetica, fontsize=9];
  
  Entry [label="processFrame() Entry\nFrame in buffer", shape=ellipse, fillcolor=lightgreen, style=filled];
  
  // Validation checks
  Check1 [label="Check 1: Frame Length\nMinimum 8 bytes?\n(Addr + FC + Data + CRC)", shape=diamond, fillcolor=lightyellow, style=filled];
  Reject1 [label="REJECT\nSilent discard\n(too short)", fillcolor=lightcoral, style=filled];
  
  Check2 [label="Check 2: CRC-16\nCalculated CRC matches\nreceived CRC?", shape=diamond, fillcolor=lightyellow, style=filled];
  Reject2 [label="REJECT\nIncrement bus error counter\nSilent discard", fillcolor=lightcoral, style=filled];
  
  Check3 [label="Check 3: Slave ID\nMatches our ID or\nBroadcast (0)?", shape=diamond, fillcolor=lightyellow, style=filled];
  Reject3 [label="REJECT\nSilent discard\n(not for us)", fillcolor=lightcoral, style=filled];
  
  IncrementBus [label="Increment bus message counter", fillcolor="#e6f2ff", style=filled];
  
  Check4 [label="Check 4: Function Code\nFC01-06, FC15-16\nsupported?", shape=diamond, fillcolor=lightyellow, style=filled];
  CallUnknown [label="Call\nonUnknownFunction()\nif registered", fillcolor=orange, style=filled];
  SendException [label="Send Exception Response\n(0x80 + FC, Code 0x01)", fillcolor=lightcoral, style=filled];
  
  IncrementSlave [label="Increment slave message counter", fillcolor="#e6f2ff", style=filled];
  
  Check5 [label="Check 5: Data Validation\n- Address in range?\n- Quantity valid?\n- Register exists?", shape=diamond, fillcolor=lightyellow, style=filled];
  SendException2 [label="Send Exception Response\n(Code 0x02: Illegal Address or\nCode 0x03: Illegal Value)", fillcolor=lightcoral, style=filled];
  
  // Action paths
  ReadAction [label="READ Operations\n(FC01-04)", fillcolor="#ccffcc", style=filled];
  CallOnRead [label="Call onRead() callbacks\nfor each address", fillcolor=orange, style=filled];
  CollectData [label="Collect register values\nfrom Register Bank", fillcolor="#e6f2ff", style=filled];
  
  WriteAction [label="WRITE Operations\n(FC05-06, FC15-16)", fillcolor="#ccffcc", style=filled];
  UpdateBank [label="Write values to\nRegister Bank", fillcolor="#e6f2ff", style=filled];
  CallOnWrite [label="Call onWrite() callbacks\nfor each address", fillcolor=orange, style=filled];
  NormalizeCoils [label="Normalize coil values\n(0x00FF or 0x0000)", fillcolor="#ffffcc", style=filled];
  
  BuildResponse [label="Build Response Frame\n- Echo request (writes)\n- Pack data (reads)\n- Calculate CRC", fillcolor="#ccffcc", style=filled];
  
  CheckBroadcast [label="Was it broadcast\n(ID=0)?", shape=diamond, fillcolor=lightyellow, style=filled];
  SendResponse [label="Send Response\n- TX Enable (RS485)\n- Write bytes\n- TX Disable", fillcolor="#ccffcc", style=filled];
  SilentSuccess [label="Silent success\n(no response to broadcast)", fillcolor="#e6ffe6", style=filled];
  
  Exit [label="Return to IDLE state", shape=ellipse, fillcolor=lightgreen, style=filled];
  
  // Flow
  Entry -> Check1;
  Check1 -> Reject1 [label="No"];
  Check1 -> Check2 [label="Yes"];
  Reject1 -> Exit;
  
  Check2 -> Reject2 [label="No"];
  Check2 -> Check3 [label="Yes"];
  Reject2 -> Exit;
  
  Check3 -> Reject3 [label="No"];
  Check3 -> IncrementBus [label="Yes"];
  Reject3 -> Exit;
  
  IncrementBus -> Check4;
  Check4 -> CallUnknown [label="No"];
  Check4 -> IncrementSlave [label="Yes"];
  CallUnknown -> SendException;
  SendException -> Exit;
  
  IncrementSlave -> Check5;
  Check5 -> SendException2 [label="No"];
  Check5 -> ReadAction [label="Yes (Read)"];
  Check5 -> WriteAction [label="Yes (Write)"];
  SendException2 -> Exit;
  
  // Read path
  ReadAction -> CallOnRead;
  CallOnRead -> CollectData;
  CollectData -> BuildResponse;
  
  // Write path
  WriteAction -> UpdateBank;
  UpdateBank -> NormalizeCoils;
  NormalizeCoils -> CallOnWrite;
  CallOnWrite -> BuildResponse;
  
  BuildResponse -> CheckBroadcast;
  CheckBroadcast -> SendResponse [label="No (unicast)"];
  CheckBroadcast -> SilentSuccess [label="Yes"];
  SendResponse -> Exit;
  SilentSuccess -> Exit;
}
@enddot

**Validation Chain Details:**

### Check 1: Frame Length

- **Minimum:** 8 bytes (Slave ID + FC + 2 data + 2 CRC)
- **Action on Fail:** Silent discard (prevents buffer underrun)

### Check 2: CRC-16 Integrity

- **Algorithm:** CRC-16-ANSI/Modbus (polynomial 0xA001)
- **Action on Fail:** Increment bus error counter, silent discard
- **Why:** Corrupted frames must not execute

### Check 3: Slave ID Matching

- **Valid IDs:** Our configured ID (1-247) or broadcast (0)
- **Action on Fail:** Silent discard (not addressed to us)
- **Multi-drop:** Prevents crosstalk on shared RS485 bus

### Check 4: Function Code Support

- **Supported:** FC01-06 (standard), FC15-16 (multi-write), FC08 (diagnostics)
- **Custom Handler:** `onUnknownFunction()` callback for vendor extensions
- **Action on Fail:** Send exception 0x01 (Illegal Function)

### Check 5: Data Validation

- **Address Range:** Register must exist in bank (`has()` check)
- **Quantity Limits:** Maximum 2000 coils, 125 registers
- **Data Bounds:** Values must be valid for register type
- **Action on Fail:** Send exception 0x02 (Illegal Address) or 0x03 (Illegal Value)

**Callback Execution Order:**

1. **onRead()** Fires before reading register values (just-in-time sampling)
2. **onWrite()** Fires after writing register values (validation/reaction)

**Broadcast Behavior:**

- Broadcasts (Slave ID = 0) are processed normally
- **No response sent** to broadcast requests (per Modbus spec)
- Counters still increment for diagnostics

---

## Security and Robustness Features {#security}

### Frame Validation

- **Silent Discard:** Invalid frames never trigger responses
- **Exception Responses:** Valid frames with bad data send proper error codes
- **Error Counting:** Diagnostic counters track communication quality

### Buffer Safety

- **Overflow Detection:** `_rxOverflow` flag prevents buffer overrun
- **Bounds Checking:** Array access validated before use
- **Length Guards:** Dynamic length checks prevent out-of-bounds reads

### Timing Robustness

- **Microsecond Precision:** Uses `micros()` for accurate t3.5 detection
- **Baud-Aware Silence:** Calculates t3.5 based on configured baud rate
- **Wraparound Handling:** Properly handles `micros()` 70-minute rollover

### Atomic Protection

- **Critical Sections:** `atomicBegin()`/`atomicEnd()` defer frame processing
- **Read-Modify-Write Safety:** Prevents master writes during application RMW cycles
- **Lock Detection:** `run()` checks atomic lock before processing

---

## Debugging Guide {#debugging}

If a Modbus master isn't receiving expected responses, trace through the validation chain:

| Symptom | Failed Check | Solution |
| ------- | ------------ | -------- |
| No response at all | Check 1 (Length) or Check 2 (CRC) | Verify baud rate, parity, stop bits match |
| Response to wrong slave | Check 3 (Slave ID) | Verify slave ID configuration matches master |
| Exception 0x01 | Check 4 (Function Code) | Verify master is using supported function codes |
| Exception 0x02/0x03 | Check 5 (Data Validation) | Verify register addresses exist in register bank |
| Intermittent failures | Buffer overflow or timing | Check for `_rxOverflow` flag, verify t3.5 timing |

**Diagnostic Counter APIs:**

```cpp
slave.getBusMessageCount();           // Total valid frames seen on bus
slave.getBusCommunicationErrorCount(); // CRC failures
slave.getSlaveMessageCount();         // Frames addressed to this slave
```

---

## Performance Characteristics {#performance}

### Execution Time (Arduino Mega 16MHz)

- **run() - IDLE state:** ~5 µs (no bytes available)
- **run() - RECEIVING state:** ~15 µs per byte
- **processFrame() - FC03 single register:** ~200 µs
- **processFrame() - FC16 multiple registers:** ~50 µs per register

### Memory Footprint

- **Static RAM:** ~350 bytes (objects + buffers)
- **Frame Buffer:** 256 bytes
- **Register Storage:** Dynamic (malloc or static pool)

### Communication Limits

- **Baud Rates:** 300 - 115200 (tested)
- **t3.5 Accuracy:** ±50 µs at 115200 baud
- **Frame Processing:** <1ms for typical operations

---

*This architecture ensures predictable, deterministic behavior suitable for industrial automation and mission-critical applications.*

---

## Author & Credits {#credits}

### Lead Developer

**Nadeem Abbas**  
📧 <zahid_printers@gmail.com>  
📱 +923924509295  
📍 Raiwind, Lahore, Pakistan

### Development Tools

This library was enhanced and professionally documented with AI assistance:

- **Google Gemini** - Architecture design, code refinement, and technical decisions
- **GitHub Copilot** - Intelligent code completion and documentation generation

### Special Acknowledgment

**Jason Vreeland [CodeRage]**  
Original Beta Library - Released March 7, 2010 under GNU License

This library is inspired by and builds upon Jason's original Beta work. The foundational concepts have been extensively modernized and enhanced to meet industrial automation standards. Key improvements include:

- **Security:** 5-layer validation chain with CRC-16 integrity checks
- **Robustness:** Non-blocking state machine with overflow protection
- **Performance:** Microsecond-precision RTU frame timing
- **Safety:** Atomic transaction API for read-modify-write protection
- **Documentation:** Bosch/Siemens-quality technical documentation with UML diagrams
- **Functionality:** Extended function code support (FC15/FC16) and callback system

*Jason's pioneering work provided the conceptual foundation that made this industrial-grade library possible. We extend our sincere gratitude for his contribution to the Arduino Modbus community.*

### Contributing

**Repository:** [https://github.com/nadeemraiwind/arduino-modbus-rtu-slave](https://github.com/nadeemraiwind/arduino-modbus-rtu-slave)

**Report Issues:** [GitHub Issues](https://github.com/nadeemraiwind/arduino-modbus-rtu-slave/issues)  
**Discuss Features:** [GitHub Discussions](https://github.com/nadeemraiwind/arduino-modbus-rtu-slave/discussions)

Contributions are welcome! See [README.md](README.md) for contribution guidelines.
