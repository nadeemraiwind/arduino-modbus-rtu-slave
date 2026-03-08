// Modbus RTU Slave - Callbacks + FC08 Diagnostics Example
// Shows active onRead/onWrite hooks and diagnostics counters.

#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

modbusDevice regBank;
modbusSlave slave;

static word speedSetpoint = 1200;
static word tempX10 = 245;

// Called when master reads input register 30001.
word onReadTemperature(word address, modbusDevice *dev) {
  (void)address;
  (void)dev;

  // Update dynamically on demand.
  if (tempX10 < 300) tempX10++;
  return tempX10;
}

// Called after master writes holding register 40001.
void onWriteSetpoint(word address, word value, modbusDevice *dev) {
  (void)address;
  speedSetpoint = value;

  // Mirror the latest setpoint into 40002 so master can verify quickly.
  if (dev->has(40002)) {
    dev->set(40002, value);
  }
}

void setup() {
  regBank.setId(1);

  // Input register with active read callback.
  regBank.add(30001);
  regBank.set(30001, tempX10);

  // Holding registers with write callback.
  regBank.add(40001);
  regBank.add(40002);
  regBank.set(40001, speedSetpoint);
  regBank.set(40002, speedSetpoint);

  slave.setDevice(&regBank);
  slave.setPort(Serial);
  slave.setBaud(9600);

  // Register callback hooks.
  slave.onRead(30001, onReadTemperature);
  slave.onWrite(40001, onWriteSetpoint);
}

void loop() {
  slave.run();

  // Optional: expose diagnostics counts in holding registers if needed.
  // Example FC08 counters available through API:
  // word busMsgs = slave.getBusMessageCount();
  // word crcErrs = slave.getBusCommunicationErrorCount();
  // word slaveMsgs = slave.getSlaveMessageCount();
  // (void)busMsgs;
  // (void)crcErrs;
  // (void)slaveMsgs;
}
