#include <modbusSlave.h>
#include <modbus.h>
#include <modbusDevice.h>
#include <Arduino.h>
#include <string.h>

/* Table of CRC values for high-order byte */
const byte _auchCRCHi[] = {
				0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
				0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
				0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
				0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
				0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
				0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
				0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
				0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
				0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
				0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
				0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
				0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
				0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
				0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
				0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
				0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
				0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
				0x40};

/* Table of CRC values for low-order byte */
const byte _auchCRCLo[] = {
				0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
				0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
				0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
				0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
				0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
				0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
				0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
				0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
				0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
				0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
				0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
				0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
				0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
				0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
				0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
				0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
				0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
				0x40};

modbusSlave::modbusSlave()
{
  _device = 0;
  _port = &Serial;
  _hwPort = &Serial;
  _len = 0;
  _baud = SERIALBAUD;
  _crc = 0;
  // Modbus RTU timing: >19200 baud uses fixed 1.75 ms, otherwise 3.5 char times.
  if(_baud > 19200)
    _frameDelayUs = 1750UL;
  else
    _frameDelayUs = 38500000UL / _baud;
  _txEnablePin = 0xFF;
  _txEnableActiveHigh = true;
  _txEnablePreDelayMs = 0;
  _txEnablePostDelayMs = 0;
  _txEnablePreDelayUs = 0;
  _txEnablePostDelayUs = 0;
  _busMessageCount = 0;
  _busCommunicationErrorCount = 0;
  _slaveMessageCount = 0;
  _unknownFunctionCallback = 0;
  _parserState = PARSER_IDLE;
  _rxIndex = 0;
  _rxOverflow = false;
  _lastRxByteUs = 0;

  for(byte i = 0; i < MODBUS_MAX_READ_CALLBACKS; i++)
  {
    _readHooks[i].address = 0;
    _readHooks[i].cb = 0;
  }

  for(byte i = 0; i < MODBUS_MAX_WRITE_CALLBACKS; i++)
  {
    _writeHooks[i].address = 0;
    _writeHooks[i].cb = 0;
  }
}

void modbusSlave::setDevice(modbusDevice *device)
{
  _device = device;
}

modbusDevice * modbusSlave::getDevice(void)
{
  return _device;
}

void modbusSlave::setPort(HardwareSerial &port)
{
  _port = &port;
  _hwPort = &port;
}

void modbusSlave::setPort(Stream &port)
{
  _port = &port;
  _hwPort = 0;
}

void modbusSlave::setTxEnablePin(byte pin, bool activeHigh)
{
  _txEnablePin = pin;
  _txEnableActiveHigh = activeHigh;
  pinMode(_txEnablePin, OUTPUT);
  digitalWrite(_txEnablePin, _txEnableActiveHigh ? LOW : HIGH);
}

void modbusSlave::setTxEnableDelays(word preDelayMs, word postDelayMs)
{
  _txEnablePreDelayMs = preDelayMs;
  _txEnablePostDelayMs = postDelayMs;
}

void modbusSlave::setTxEnableDelaysUs(word preDelayUs, word postDelayUs)
{
  _txEnablePreDelayUs = preDelayUs;
  _txEnablePostDelayUs = postDelayUs;
}

bool modbusSlave::onRead(word address, modbusReadCallback cb)
{
  if(cb == 0)
    return false;

  for(byte i = 0; i < MODBUS_MAX_READ_CALLBACKS; i++)
  {
    if((_readHooks[i].cb != 0) && (_readHooks[i].address == address))
    {
      _readHooks[i].cb = cb;
      return true;
    }
  }

  for(byte i = 0; i < MODBUS_MAX_READ_CALLBACKS; i++)
  {
    if(_readHooks[i].cb == 0)
    {
      _readHooks[i].address = address;
      _readHooks[i].cb = cb;
      return true;
    }
  }

  return false;
}

bool modbusSlave::onWrite(word address, modbusWriteCallback cb)
{
  if(cb == 0)
    return false;

  for(byte i = 0; i < MODBUS_MAX_WRITE_CALLBACKS; i++)
  {
    if((_writeHooks[i].cb != 0) && (_writeHooks[i].address == address))
    {
      _writeHooks[i].cb = cb;
      return true;
    }
  }

  for(byte i = 0; i < MODBUS_MAX_WRITE_CALLBACKS; i++)
  {
    if(_writeHooks[i].cb == 0)
    {
      _writeHooks[i].address = address;
      _writeHooks[i].cb = cb;
      return true;
    }
  }

  return false;
}

bool modbusSlave::onUnknownFunction(modbusUnknownFunctionCallback cb)
{
  _unknownFunctionCallback = cb;
  return true;
}

void modbusSlave::configureEndianness(byte mode)
{
  if(_device)
    _device->setEndianness(mode);
}

byte modbusSlave::getEndianness(void)
{
  if(_device)
    return _device->getEndianness();
  return MODBUS_BIG_ENDIAN;
}

word modbusSlave::getBusMessageCount(void)
{
  return _busMessageCount;
}

word modbusSlave::getBusCommunicationErrorCount(void)
{
  return _busCommunicationErrorCount;
}

word modbusSlave::getSlaveMessageCount(void)
{
  return _slaveMessageCount;
}

void modbusSlave::clearDiagnosticsCounters(void)
{
  _busMessageCount = 0;
  _busCommunicationErrorCount = 0;
  _slaveMessageCount = 0;
}

/*
Set the Serial Baud rate.
Reconfigure the UART for 8 data bits, no parity, and 1 stop bit.
For non-HardwareSerial Stream transports, caller must configure transport speed.
*/
void modbusSlave::setBaud(word baud)
{
  if(_port == 0)
    return;

  _baud = baud;
  // Modbus RTU timing: >19200 baud uses fixed 1.75 ms, otherwise 3.5 char times.
  if(_baud > 19200)
    _frameDelayUs = 1750UL;
  else
    _frameDelayUs = 38500000UL / _baud;

  if(_hwPort)
    _hwPort->begin(baud);
  _port->flush();
}

/*
Retrieve the serial baud rate
*/
word modbusSlave::getBaud(void)
{
  return(_baud);
}

/*
Generates the crc for the current message in the buffer.
*/
void modbusSlave::calcCrc(void)
{
  byte CRCHi = 0xFF,
     CRCLo = 0x0FF,
     Index,
     msgLen,
     *msgPtr;

  msgLen = _len - 2;
  msgPtr = _msg;

  while(msgLen--)
  {
    Index = CRCHi ^ *msgPtr++;
    CRCHi = CRCLo ^ _auchCRCHi[Index];
    CRCLo = _auchCRCLo[Index];
  }
  // Table variables are historically named Hi/Lo but hold swapped roles here.
  // Build canonical CRC word so low byte is (_crc & 0xFF), high is (_crc >> 8).
  _crc = (CRCLo << 8) | CRCHi;
}

/*
  Checks the UART for query data
*/
void modbusSlave::checkSerial(void)
{
  if(_port == 0)
    return;

  if(_port->available() <= 0)
    return;

  // Wait until byte count is stable for >= 3.5 character times (RTU framing).
  byte lastCount = (byte) _port->available();
  unsigned long lastChangeUs = micros();
  unsigned long silenceWindowUs = (_frameDelayUs < 300UL) ? 300UL : _frameDelayUs;

  while((micros() - lastChangeUs) < silenceWindowUs)
  {
    byte currentCount = (byte) _port->available();
    if(currentCount != lastCount)
    {
      lastCount = currentCount;
      lastChangeUs = micros();
    }
    delayMicroseconds(50);
  }

  _len = (lastCount > MODBUS_MAX_FRAME) ? MODBUS_MAX_FRAME : lastCount;
}

/*
Copies the contents of the UART to the internal buffer
*/
void modbusSlave::serialRx(void)
{
  byte i;

  if(_port == 0)
    return;

  if(_len > MODBUS_MAX_FRAME)
    _len = MODBUS_MAX_FRAME;

  if(_len == 0)
    return;

  for (i = 0; i < _len; i++)
  {
    int r = _port->read();
    if(r < 0)
    {
      _len = 0;
      return;
    }
    _msg[i] = (byte)r;
  }
}

bool modbusSlave::isSupportedFunction(byte funcType)
{
  return (funcType == READ_DI) ||
      (funcType == READ_DO) ||
      (funcType == READ_AI) ||
      (funcType == READ_AO) ||
      (funcType == WRITE_DO) ||
  (funcType == WRITE_AO) ||
  (funcType == WRITE_DO_MULTI) ||
  (funcType == WRITE_AO_MULTI) ||
  (funcType == DIAGNOSTICS);
}

modbusReadCallback modbusSlave::findReadCallback(word address)
{
  for(byte i = 0; i < MODBUS_MAX_READ_CALLBACKS; i++)
  {
    if((_readHooks[i].cb != 0) && (_readHooks[i].address == address))
      return _readHooks[i].cb;
  }

  return 0;
}

modbusWriteCallback modbusSlave::findWriteCallback(word address)
{
  for(byte i = 0; i < MODBUS_MAX_WRITE_CALLBACKS; i++)
  {
    if((_writeHooks[i].cb != 0) && (_writeHooks[i].address == address))
      return _writeHooks[i].cb;
  }

  return 0;
}

word modbusSlave::invokeReadCallback(word address, word fallbackValue)
{
  modbusReadCallback cb = findReadCallback(address);
  if(cb == 0)
    return fallbackValue;

  return cb(address, _device);
}

void modbusSlave::invokeWriteCallback(word address, word value)
{
  modbusWriteCallback cb = findWriteCallback(address);
  if(cb)
    cb(address, value, _device);
}

bool modbusSlave::validateRange(word startAddr, word count)
{
  word i;
  for(i = 0; i < count; i++)
  {
    if(!_device->has(startAddr + i))
      return false;
  }
  return true;
}

byte modbusSlave::validateRequest(byte funcType, word field1, word field2, byte requestLen)
{
  word startAddr;
  byte byteCount;
  byte expectedBytes;

  switch(funcType)
  {
  case READ_DO:  // FC01 - Read Coils (1-2000)
    if(requestLen != 8)
      return EX_ILLEGAL_DATA_VALUE;
    if((field2 < 1) || (field2 > 2000))
      return EX_ILLEGAL_DATA_VALUE;
    // Try to validate range, but if it fails, attempt single registers
    // This allows reading ANY coils that have been added
    for(word i = 0; i < field2; i++)
    {
      if(!_device->has(field1 + 1 + i))
        return EX_ILLEGAL_DATA_ADDRESS;
    }
    break;
  case READ_DI:  // FC02 - Read Discrete Inputs (10001-12000)
    if(requestLen != 8)
      return EX_ILLEGAL_DATA_VALUE;
    if((field2 < 1) || (field2 > 2000))
      return EX_ILLEGAL_DATA_VALUE;
    for(word i = 0; i < field2; i++)
    {
      if(!_device->has(field1 + 10001 + i))
        return EX_ILLEGAL_DATA_ADDRESS;
    }
    break;
  case READ_AI:  // FC04 - Read Input Registers (30001-30125)
    if(requestLen != 8)
      return EX_ILLEGAL_DATA_VALUE;
    if((field2 < 1) || (field2 > 125))
      return EX_ILLEGAL_DATA_VALUE;
    for(word i = 0; i < field2; i++)
    {
      if(!_device->has(field1 + 30001 + i))
        return EX_ILLEGAL_DATA_ADDRESS;
    }
    break;
  case READ_AO:  // FC03 - Read Holding Registers (40001-40125)
    if(requestLen != 8)
      return EX_ILLEGAL_DATA_VALUE;
    if((field2 < 1) || (field2 > 125))
      return EX_ILLEGAL_DATA_VALUE;
    for(word i = 0; i < field2; i++)
    {
      if(!_device->has(field1 + 40001 + i))
        return EX_ILLEGAL_DATA_ADDRESS;
    }
    break;
  case WRITE_DO:  // FC05 - Write Single Coil
    if(requestLen != 8)
      return EX_ILLEGAL_DATA_VALUE;
    if((field2 != 0x0000) && (field2 != 0xFF00))
      return EX_ILLEGAL_DATA_VALUE;
    startAddr = field1 + 1;
    if(!_device->has(startAddr))
      return EX_ILLEGAL_DATA_ADDRESS;
    break;
  case WRITE_AO:  // FC06 - Write Single Holding Register
    if(requestLen != 8)
      return EX_ILLEGAL_DATA_VALUE;
    startAddr = field1 + 40001;
    if(!_device->has(startAddr))
      return EX_ILLEGAL_DATA_ADDRESS;
    break;
  case WRITE_DO_MULTI:  // FC15 - Write Multiple Coils
    if(requestLen < 9)
      return EX_ILLEGAL_DATA_VALUE;
    if((field2 < 1) || (field2 > 1968))
      return EX_ILLEGAL_DATA_VALUE;
    byteCount = _msg[6];
    expectedBytes = (field2 + 7) / 8;
    if(byteCount != expectedBytes)
      return EX_ILLEGAL_DATA_VALUE;
    if(requestLen != (byte)(9 + byteCount))
      return EX_ILLEGAL_DATA_VALUE;
    startAddr = field1 + 1;
    if(!validateRange(startAddr, field2))
      return EX_ILLEGAL_DATA_ADDRESS;
    break;
  case WRITE_AO_MULTI:  // FC16 - Write Multiple Holding Registers
    if(requestLen < 9)
      return EX_ILLEGAL_DATA_VALUE;
    if((field2 < 1) || (field2 > 123))
      return EX_ILLEGAL_DATA_VALUE;
    byteCount = _msg[6];
    expectedBytes = field2 * 2;
    if(byteCount != expectedBytes)
      return EX_ILLEGAL_DATA_VALUE;
    if(requestLen != (byte)(9 + byteCount))
      return EX_ILLEGAL_DATA_VALUE;
    startAddr = field1 + 40001;
    if(!validateRange(startAddr, field2))
      return EX_ILLEGAL_DATA_ADDRESS;
    break;
  case DIAGNOSTICS:  // FC08 - Diagnostics
    if(requestLen != 8)
      return EX_ILLEGAL_DATA_VALUE;
    if((field1 == DIAG_RETURN_QUERY_DATA) ||
       (field1 == DIAG_BUS_MESSAGE_COUNT) ||
       (field1 == DIAG_BUS_COMM_ERROR_COUNT) ||
       (field1 == DIAG_SLAVE_MESSAGE_COUNT))
      break;
    return EX_ILLEGAL_DATA_VALUE;
  default:
    return EX_ILLEGAL_FUNCTION;
  }

  return 0;
}

void modbusSlave::sendException(byte funcType, byte exceptionCode)
{
  _len = 5;
  _msg[0] = _device->getId();
  _msg[1] = funcType | 0x80;
  _msg[2] = exceptionCode;

  this->calcCrc();
  _msg[_len - 2] = _crc & 0xFF;
  _msg[_len - 1] = _crc >> 8;

  this->sendFrame();
  _len = 0;
}

void modbusSlave::sendFrame(void)
{
  if(_port == 0)
    return;

  if(_txEnablePin != 0xFF)
  {
    digitalWrite(_txEnablePin, _txEnableActiveHigh ? HIGH : LOW);
    if(_txEnablePreDelayMs)
      delay(_txEnablePreDelayMs);
    if(_txEnablePreDelayUs)
      delayMicroseconds(_txEnablePreDelayUs);
  }

  _port->write(_msg, _len);
  _port->flush();

  if(_txEnablePin != 0xFF)
  {
    if(_txEnablePostDelayUs)
      delayMicroseconds(_txEnablePostDelayUs);
    if(_txEnablePostDelayMs)
      delay(_txEnablePostDelayMs);
    digitalWrite(_txEnablePin, _txEnableActiveHigh ? LOW : HIGH);
  }
}

/*
Generates a query reply message for Digital In/Out status update queries.
Supports FC01 (Read Coils) and FC02 (Read Discrete Inputs)
*/
void modbusSlave::getDigitalStatus(byte funcType, word startreg, word numregs)
{
  word bitn = 0;
  word i;
  byte dataBytes;
  word actualReg;

  // Calculate how many bytes needed (1 byte = 8 bits)
  dataBytes = numregs / 8;
  if(numregs % 8)
    dataBytes++;

  // Check frame size won't overflow
  _len = dataBytes + 5;  // 1(ID) + 1(FC) + 1(ByteCount) + dataBytes + 2(CRC)
  if(_len > MODBUS_MAX_FRAME)
  {
    _len = 0;
    return;
  }

  // Build response frame
  _msg[0] = _device->getId();
  _msg[1] = funcType;
  _msg[2] = dataBytes;
  
  // Clear all data bytes
  for(byte j = 0; j < dataBytes; j++)
    _msg[3 + j] = 0;

  // For each requested register, get its value and set corresponding bit
  for(i = 0; i < numregs; i++)
  {
    // Determine actual address (depends on function type)
    if(funcType == READ_DI)  // FC02 - Discrete Inputs (10001-10005)
      actualReg = startreg + 10001 + i;
    else  // FC01 - Coils (1-5)
      actualReg = startreg + 1 + i;

    byte outByte = 3 + (bitn / 8);
    byte outBit = bitn % 8;

    // Get value from register and set bit if non-zero
    word regValue = invokeReadCallback(actualReg, _device->get(actualReg));

    if(regValue)
      bitSet(_msg[outByte], outBit);
    else
      bitClear(_msg[outByte], outBit);

    bitn++;
  }

  // Calculate and append CRC
  this->calcCrc();
  _msg[_len - 2] = _crc & 0xFF;
  _msg[_len - 1] = _crc >> 8;
}

/*
Generates a query reply message for Analog In/Out status update queries.
Supports FC03 (Read Holding Registers) and FC04 (Read Input Registers)
*/
void modbusSlave::getAnalogStatus(byte funcType, word startreg, word numregs)
{
  word val;
  word i = 0;
  word actualReg;

  // Check frame size won't overflow: 1(ID) + 1(FC) + 1(ByteCount) + 2*numregs + 2(CRC)
  _len = (numregs * 2) + 5;
  if(_len > MODBUS_MAX_FRAME)
  {
    _len = 0;
    return;
  }

  // Build response frame
  _msg[0] = _device->getId();
  _msg[1] = funcType;
  _msg[2] = numregs * 2;  // Byte count (each register is 2 bytes)

  // For each requested register, get value and add to response
  while(numregs--)
  {
    // Determine actual address (depends on function type)
    if(funcType == READ_AI)  // FC04 - Input Registers (30001-30005)
      actualReg = startreg + 30001 + i;
    else  // FC03 - Holding Registers (40001-40005)
      actualReg = startreg + 40001 + i;

    // Get register value
    val = invokeReadCallback(actualReg, _device->get(actualReg));
    
    // Append as Big-Endian (high byte first, low byte second) - Modbus standard
    _msg[3 + i * 2] = val >> 8;        // High byte
    _msg[4 + i * 2] = val & 0xFF;     // Low byte
    
    i++;
  }

  // Calculate and append CRC
  this->calcCrc();
  _msg[_len - 2] = _crc & 0xFF;
  _msg[_len - 1] = _crc >> 8;
}

/*
Sends response for Write operations (FC05 Write Coil, FC06 Write Holding Register)
*/
void modbusSlave::setStatus(byte funcType, word reg, word val)
{
  _len = 8;  // Fixed 8-byte response for write operations

  _msg[0] = _device->getId();
  _msg[1] = funcType;
  _msg[2] = reg >> 8;
  _msg[3] = reg & 0xFF;
  
  if(funcType == WRITE_DO)  // FC05 - Write Single Coil (addresses 1-2000)
  {
    word normalized = (val != 0) ? 0x00FF : 0x0000;
    // Store coil at: 1 + reg
    _device->set(reg + 1, val);
    invokeWriteCallback(reg + 1, normalized);
    _msg[4] = val >> 8;
    _msg[5] = val & 0xFF;
  }
  else  // FC06 - Write Single Holding Register (addresses 40001-40125)
  {
    // Store at: 40001 + reg
    _device->set(reg + 40001, val);
    invokeWriteCallback(reg + 40001, val);
    _msg[4] = val >> 8;
    _msg[5] = val & 0xFF;
  }

  this->calcCrc();
  _msg[_len - 2] = _crc & 0xFF;
  _msg[_len - 1] = _crc >> 8;
}

/*
Sends response for multi-write operations (FC15, FC16)
*/
void modbusSlave::setStatusMultiple(byte funcType, word reg, word count)
{
  word i;

  if(funcType == WRITE_DO_MULTI)
  {
    // Request data starts at byte 7, each bit is one coil value.
    for(i = 0; i < count; i++)
    {
      byte packed = _msg[7 + (i / 8)];
      word value = (packed & (1 << (i % 8))) ? 0xFF : 0x0000;
      _device->set(reg + 1 + i, value);
      invokeWriteCallback(reg + 1 + i, value);
    }
  }
  else
  {
    // FC16 payload is big-endian words from byte 7 onward.
    for(i = 0; i < count; i++)
    {
      word value = ((word)_msg[7 + (i * 2)] << 8) | _msg[8 + (i * 2)];
      _device->set(reg + 40001 + i, value);
      invokeWriteCallback(reg + 40001 + i, value);
    }
  }

  // Standard Modbus write-multiple response: echo start + quantity.
  _len = 8;
  _msg[0] = _device->getId();
  _msg[1] = funcType;
  _msg[2] = reg >> 8;
  _msg[3] = reg & 0xFF;
  _msg[4] = count >> 8;
  _msg[5] = count & 0xFF;

  this->calcCrc();
  _msg[_len - 2] = _crc & 0xFF;
  _msg[_len - 1] = _crc >> 8;
}

void modbusSlave::diagnostics(word subFunction, word data)
{
  _len = 8;
  _msg[0] = _device->getId();
  _msg[1] = DIAGNOSTICS;
  _msg[2] = subFunction >> 8;
  _msg[3] = subFunction & 0xFF;

  switch(subFunction)
  {
  case DIAG_RETURN_QUERY_DATA:
    _msg[4] = data >> 8;
    _msg[5] = data & 0xFF;
    break;
  case DIAG_BUS_MESSAGE_COUNT:
    _msg[4] = _busMessageCount >> 8;
    _msg[5] = _busMessageCount & 0xFF;
    break;
  case DIAG_BUS_COMM_ERROR_COUNT:
    _msg[4] = _busCommunicationErrorCount >> 8;
    _msg[5] = _busCommunicationErrorCount & 0xFF;
    break;
  case DIAG_SLAVE_MESSAGE_COUNT:
    _msg[4] = _slaveMessageCount >> 8;
    _msg[5] = _slaveMessageCount & 0xFF;
    break;
  default:
    _len = 0;
    return;
  }

  this->calcCrc();
  _msg[_len - 2] = _crc & 0xFF;
  _msg[_len - 1] = _crc >> 8;
}

void modbusSlave::resetParser(void)
{
  _parserState = PARSER_IDLE;
  _rxIndex = 0;
  _rxOverflow = false;
  _lastRxByteUs = 0;
}

void modbusSlave::processFrame(void)
{
  byte deviceId;
  byte funcType;
  word field1;
  word field2;
  word rxCrc;
  byte exceptionCode;
  bool isBroadcast;
  byte customLen;

  if(_len < 8)
  {
    _len = 0;
    return;
  }

  _busMessageCount++;

  deviceId = _msg[0];
  isBroadcast = (deviceId == 0);

  this->calcCrc();
  rxCrc = ((word)_msg[_len - 1] << 8) | _msg[_len - 2];
  if(_crc != rxCrc)
  {
    _busCommunicationErrorCount++;
    _len = 0;
    return;
  }

  if((deviceId != _device->getId()) && !isBroadcast)
  {
    _len = 0;
    return;
  }

  if(deviceId == _device->getId())
    _slaveMessageCount++;

  funcType = _msg[1];
  field1 = ((word)_msg[2] << 8) | _msg[3];
  field2 = ((word)_msg[4] << 8) | _msg[5];

  if(!isSupportedFunction(funcType))
  {
    if(!isBroadcast && _unknownFunctionCallback)
    {
      byte requestCopy[MODBUS_MAX_FRAME];
      memcpy(requestCopy, _msg, _len);
      customLen = 0;
      if(_unknownFunctionCallback(funcType, requestCopy, _len, _msg, &customLen, _device))
      {
        if(customLen <= MODBUS_MAX_FRAME)
          _len = customLen;
        else
          _len = 0;
      }
      else
      {
        _len = 0;
      }

      if(_len)
        this->sendFrame();
    }
    else if(!isBroadcast)
    {
      sendException(funcType, EX_ILLEGAL_FUNCTION);
    }

    _len = 0;
    return;
  }

  exceptionCode = validateRequest(funcType, field1, field2, _len);
  if(exceptionCode)
  {
    if(!isBroadcast)
      sendException(funcType, exceptionCode);
    _len = 0;
    return;
  }

  _len = 0;

  switch(funcType)
  {
  case READ_DI:
  case READ_DO:
    this->getDigitalStatus(funcType, field1, field2);
    break;
  case READ_AI:
  case READ_AO:
    this->getAnalogStatus(funcType, field1, field2);
    break;
  case WRITE_DO:
  case WRITE_AO:
    this->setStatus(funcType, field1, field2);
    break;
  case WRITE_DO_MULTI:
  case WRITE_AO_MULTI:
    this->setStatusMultiple(funcType, field1, field2);
    break;
  case DIAGNOSTICS:
    this->diagnostics(field1, field2);
    break;
  default:
    return;
  }

  if(_len && !isBroadcast)
  {
    this->sendFrame();
    _len = 0;
  }
}

void modbusSlave::run(void)
{
  if((_device == 0) || (_port == 0))
    return;

  while(_port->available() > 0)
  {
    int r = _port->read();
    if(r < 0)
      break;

    if(_rxIndex < MODBUS_MAX_FRAME)
      _msg[_rxIndex++] = (byte)r;
    else
      _rxOverflow = true;

    _lastRxByteUs = micros();
    _parserState = PARSER_RECEIVING;
  }

  if(_parserState == PARSER_RECEIVING)
  {
    unsigned long nowUs = micros();
    if((nowUs - _lastRxByteUs) >= _frameDelayUs)
    {
      if(_rxOverflow)
      {
        _busCommunicationErrorCount++;
        this->resetParser();
        return;
      }

      _len = _rxIndex;
      _parserState = PARSER_COMPLETE;
    }
    else
    {
      return;
    }
  }

  if(_parserState != PARSER_COMPLETE)
    return;

  this->processFrame();
  this->resetParser();
}
