#include <stdint.h>
#include <Arduino.h>

#ifndef MODBUSSLAVE
#define MODBUSSLAVE

#include <modbus.h>
#include <modbusDevice.h>
//#include <Wprogram.h>

/* CRC lookup tables - defined in modbusSlave.cpp */
extern const byte _auchCRCHi[256];
extern const byte _auchCRCLo[256];

typedef word (*modbusReadCallback)(word address, modbusDevice *device);
typedef void (*modbusWriteCallback)(word address, word value, modbusDevice *device);
typedef bool (*modbusUnknownFunctionCallback)(byte funcType, const byte *request, byte requestLen, byte *response, byte *responseLen, modbusDevice *device);

class modbusSlave
{
	public:
		modbusSlave(void);
		void setDevice(modbusDevice *device);
		modbusDevice * getDevice(void);
		void setPort(Stream &port);
		void setPort(HardwareSerial &port);
		void setTxEnablePin(byte pin, bool activeHigh = true);
		void setTxEnableDelays(word preDelayMs, word postDelayMs);
		void setTxEnableDelaysUs(word preDelayUs, word postDelayUs);
		bool onRead(word address, modbusReadCallback cb);
		bool onWrite(word address, modbusWriteCallback cb);
		bool onUnknownFunction(modbusUnknownFunctionCallback cb);
		void configureEndianness(byte mode);
		byte getEndianness(void);
		word getBusMessageCount(void);
		word getBusCommunicationErrorCount(void);
		word getSlaveMessageCount(void);
		void clearDiagnosticsCounters(void);
		void setBaud(word);
		word getBaud(void);
		void calcCrc(void);
		void checkSerial(void);
		void serialRx(void);
		void getDigitalStatus(byte, word, word);
		void getAnalogStatus(byte, word, word);
		void setStatus(byte, word, word);
		void setStatusMultiple(byte, word, word);
		void diagnostics(word, word);
		void run(void);

	private:
		bool isSupportedFunction(byte funcType);
		byte validateRequest(byte funcType, word field1, word field2, byte requestLen);
		bool validateRange(word startAddr, word count);
		modbusReadCallback findReadCallback(word address);
		modbusWriteCallback findWriteCallback(word address);
		word invokeReadCallback(word address, word fallbackValue);
		void invokeWriteCallback(word address, word value);
		void resetParser(void);
		void processFrame(void);
		void sendException(byte funcType, byte exceptionCode);
		void sendFrame(void);

		struct modbusReadHook
		{
			word address;
			modbusReadCallback cb;
		};

		struct modbusWriteHook
		{
			word address;
			modbusWriteCallback cb;
		};

		enum ParserState
		{
			PARSER_IDLE,
			PARSER_RECEIVING,
			PARSER_COMPLETE
		};

		modbusDevice *_device;
		Stream *_port;
		HardwareSerial *_hwPort;
		byte _msg[MODBUS_MAX_FRAME],
			 _len;
		byte _txEnablePin;
		bool _txEnableActiveHigh;


		word _baud,
			 _crc,
			 _txEnablePreDelayMs,
			 _txEnablePostDelayMs,
			 _txEnablePreDelayUs,
			 _txEnablePostDelayUs,
			 _busMessageCount,
			 _busCommunicationErrorCount,
			 _slaveMessageCount;
		unsigned long _frameDelayUs;
		modbusReadHook _readHooks[MODBUS_MAX_READ_CALLBACKS];
		modbusWriteHook _writeHooks[MODBUS_MAX_WRITE_CALLBACKS];
		modbusUnknownFunctionCallback _unknownFunctionCallback;
		ParserState _parserState;
		byte _rxIndex;
		unsigned long _lastRxByteUs;
};
#endif
