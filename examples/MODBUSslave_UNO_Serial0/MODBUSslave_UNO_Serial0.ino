#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

modbusDevice regBank;
modbusSlave slave;

// Simple counters used to simulate changing process values.
static word tempX10 = 250;
static word pressureX10 = 65;
static unsigned long lastUpdateMs = 0;

void configureRegisterBank()
{
  // Coils (FC01 read, FC05 write): 00001..00005
  regBank.add(1);
  regBank.add(2);
  regBank.add(3);
  regBank.add(4);
  regBank.add(5);

  // Discrete Inputs (FC02 read): 10001..10005
  regBank.add(10001);
  regBank.add(10002);
  regBank.add(10003);
  regBank.add(10004);
  regBank.add(10005);

  // Input Registers (FC04 read): 30001..30005
  regBank.add(30001);
  regBank.add(30002);
  regBank.add(30003);
  regBank.add(30004);
  regBank.add(30005);

  // Holding Registers (FC03 read, FC06 write): 40001..40005
  regBank.add(40001);
  regBank.add(40002);
  regBank.add(40003);
  regBank.add(40004);
  regBank.add(40005);
}

void seedInitialValues()
{
  // Coils: command values written by master via FC05.
  regBank.set(1, 0);
  regBank.set(2, 0);
  regBank.set(3, 0);
  regBank.set(4, 0);
  regBank.set(5, 0);

  // Discrete Inputs: read-only status for master via FC02.
  regBank.set(10001, 0);
  regBank.set(10002, 0);
  regBank.set(10003, 0);
  regBank.set(10004, 0);
  regBank.set(10005, 0);

  // Input Registers: read-only process values for FC04.
  regBank.set(30001, tempX10);      // temperature x10
  regBank.set(30002, pressureX10);  // pressure x10
  regBank.set(30003, 520);          // humidity x10
  regBank.set(30004, 120);          // flow x10
  regBank.set(30005, 240);          // voltage x10

  // Holding Registers: writable setpoints for FC03/FC06.
  regBank.set(40001, 350);  // speed setpoint x10
  regBank.set(40002, 300);  // temperature setpoint x10
  regBank.set(40003, 500);  // valve position x10
  regBank.set(40004, 1000); // delay in ms
  regBank.set(40005, 100);  // gain x100
}

void setup()
{
  // Device-level API usage.
  regBank.setId(1);
  configureRegisterBank();
  seedInitialValues();

  // Slave-level API usage.
  slave.setDevice(&regBank);
  slave.setPort(Serial);      // Serial0 example
  slave.setBaud(9600);

  // Optional RS485 direction control API usage (disabled by default).
  // slave.setTxEnablePin(2, true);
  // slave.setTxEnableDelays(0, 1);
}

void updateProcessModel()
{
  // Update simulated sensor values every 250 ms.
  if ((millis() - lastUpdateMs) < 250)
    return;

  lastUpdateMs = millis();

  // Use coil command states as machine command inputs.
  word motor1Cmd = regBank.get(1);
  word motor2Cmd = regBank.get(2);

  // Reflect command into status bits (discrete inputs).
  regBank.set(10001, motor1Cmd ? 1 : 0);
  regBank.set(10002, motor2Cmd ? 1 : 0);
  regBank.set(10003, regBank.get(3) ? 1 : 0);
  regBank.set(10004, regBank.get(4) ? 1 : 0);
  regBank.set(10005, regBank.get(5) ? 1 : 0);

  // Simulate process trending around FC06-controlled setpoints.
  word tempSetpoint = regBank.get(40002);
  if (tempX10 < tempSetpoint)
    tempX10++;
  else if (tempX10 > tempSetpoint)
    tempX10--;

  word speedSetpoint = regBank.get(40001);
  if (speedSetpoint > 500)
    pressureX10 = 90;
  else if (speedSetpoint > 300)
    pressureX10 = 75;
  else
    pressureX10 = 60;

  // Publish process values for FC04.
  regBank.set(30001, tempX10);
  regBank.set(30002, pressureX10);
  regBank.set(30003, 500 + (tempX10 % 40));
  regBank.set(30004, 100 + (speedSetpoint / 10));
  regBank.set(30005, 235 + (regBank.get(4) ? 5 : 0));

  // Demonstrate has() before using optional register logic.
  if (regBank.has(40005) && regBank.has(30001))
  {
    word gain = regBank.get(40005);
    // Store derived control effort into holding register 40003.
    regBank.set(40003, (word)((tempSetpoint > tempX10) ? gain : (gain / 2)));
  }
}

void loop()
{
  // Demonstrate getId(), getDevice(), getBaud() usage without debug serial.
  volatile byte slaveId = regBank.getId();
  volatile modbusDevice *dev = slave.getDevice();
  volatile word baud = slave.getBaud();
  (void)slaveId;
  (void)dev;
  (void)baud;

  updateProcessModel();

  // Required: process all Modbus requests (FC01..FC06).
  slave.run();
}
