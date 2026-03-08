#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

modbusDevice regBank;
modbusSlave slave;

static word tempX10 = 240;
static word pressureX10 = 70;
static unsigned long lastUpdateMs = 0;

void configureRegisterBank()
{
  // Coils (FC01/FC05)
  regBank.add(1);
  regBank.add(2);
  regBank.add(3);
  regBank.add(4);
  regBank.add(5);

  // Discrete Inputs (FC02)
  regBank.add(10001);
  regBank.add(10002);
  regBank.add(10003);
  regBank.add(10004);
  regBank.add(10005);

  // Input Registers (FC04)
  regBank.add(30001);
  regBank.add(30002);
  regBank.add(30003);
  regBank.add(30004);
  regBank.add(30005);

  // Holding Registers (FC03/FC06)
  regBank.add(40001);
  regBank.add(40002);
  regBank.add(40003);
  regBank.add(40004);
  regBank.add(40005);
}

void seedInitialValues()
{
  regBank.set(1, 0);
  regBank.set(2, 0);
  regBank.set(3, 0);
  regBank.set(4, 0);
  regBank.set(5, 0);

  regBank.set(10001, 0);
  regBank.set(10002, 0);
  regBank.set(10003, 0);
  regBank.set(10004, 0);
  regBank.set(10005, 0);

  regBank.set(30001, tempX10);
  regBank.set(30002, pressureX10);
  regBank.set(30003, 500);
  regBank.set(30004, 120);
  regBank.set(30005, 240);

  regBank.set(40001, 320);
  regBank.set(40002, 290);
  regBank.set(40003, 450);
  regBank.set(40004, 1000);
  regBank.set(40005, 100);
}

void setup()
{
  // Device API: set slave address.
  regBank.setId(1);

  configureRegisterBank();
  seedInitialValues();

  // Slave API: bind device and configure communication.
  slave.setDevice(&regBank);
  slave.setPort(Serial1);     // Serial1 example for MEGA (RX1/TX1).
  slave.setBaud(9600);

  // Optional RS485 direction control support.
  // slave.setTxEnablePin(2, true);
  // slave.setTxEnableDelays(0, 1);
}

void updateProcessModel()
{
  if ((millis() - lastUpdateMs) < 250)
    return;

  lastUpdateMs = millis();

  // Mirror coil commands into discrete input status bits.
  regBank.set(10001, regBank.get(1) ? 1 : 0);
  regBank.set(10002, regBank.get(2) ? 1 : 0);
  regBank.set(10003, regBank.get(3) ? 1 : 0);
  regBank.set(10004, regBank.get(4) ? 1 : 0);
  regBank.set(10005, regBank.get(5) ? 1 : 0);

  // FC06 writes can change these setpoints while running.
  word tempSetpoint = regBank.get(40002);
  word speedSetpoint = regBank.get(40001);

  if (tempX10 < tempSetpoint)
    tempX10 += 2;
  else if (tempX10 > tempSetpoint)
    tempX10 -= 2;

  pressureX10 = (word)(55 + (speedSetpoint / 20));

  // Publish values for FC04 reads.
  regBank.set(30001, tempX10);
  regBank.set(30002, pressureX10);
  regBank.set(30003, 450 + (tempX10 % 80));
  regBank.set(30004, 100 + (speedSetpoint / 10));
  regBank.set(30005, 238 + (regBank.get(4) ? 2 : 0));

  // Example of has() guard around optional register operations.
  if (regBank.has(40003) && regBank.has(40005))
  {
    word gain = regBank.get(40005);
    regBank.set(40003, (word)((pressureX10 * gain) / 100));
  }
}

void loop()
{
  // API usage examples kept active but without serial debug output.
  volatile byte slaveId = regBank.getId();
  volatile modbusDevice *dev = slave.getDevice();
  volatile word baud = slave.getBaud();
  (void)slaveId;
  (void)dev;
  (void)baud;

  updateProcessModel();

  // Required main handler for Modbus RTU request processing.
  slave.run();
}
