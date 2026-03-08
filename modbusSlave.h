#include <stdint.h>
#include <Arduino.h>

#ifndef MODBUSSLAVE
#define MODBUSSLAVE

#include <modbus.h>
#include <modbusDevice.h>

/**
 * @file modbusSlave.h
 * @brief Modbus RTU slave protocol engine and callback API.
 */

/** @defgroup rs485_control RS485 Control
 * RS485 DE/RE direction control and timing behavior.
 * @{
 */

/** @brief CRC lookup high-byte table, defined in modbusSlave.cpp. */
extern const byte _auchCRCHi[256];
/** @brief CRC lookup low-byte table, defined in modbusSlave.cpp. */
extern const byte _auchCRCLo[256];

/** @brief Callback signature for dynamic read values. */
typedef word (*modbusReadCallback)(word address, modbusDevice *device);
/** @brief Callback signature invoked after register writes. */
typedef void (*modbusWriteCallback)(word address, word value, modbusDevice *device);
/** @brief Callback signature for custom/unknown function code handling. */
typedef bool (*modbusUnknownFunctionCallback)(byte funcType, const byte *request, byte requestLen, byte *response, byte *responseLen, modbusDevice *device);

/**
 * @class modbusSlave
 * @brief Modbus RTU slave communications and frame processing engine.
 * @ingroup protocol_engine
 */
class modbusSlave
{
	public:
		/** @brief Construct a slave instance with defaults. */
		modbusSlave(void);
		/** @brief Bind logical register device model. */
		void setDevice(modbusDevice *device);
		/** @brief Get currently bound device model pointer. */
		inline modbusDevice * getDevice(void) { return _device; }
		/** @brief Bind generic Stream transport. */
		void setPort(Stream &port);
		/** @brief Bind hardware serial transport. */
		void setPort(HardwareSerial &port);
		/** @brief Configure RS485 DE/RE direction pin control. */
		/** @ingroup rs485_control */
		void setTxEnablePin(byte pin, bool activeHigh = true);
		/** @brief Configure RS485 ms-level pre/post TX delays. */
		/** @ingroup rs485_control */
		void setTxEnableDelays(word preDelayMs, word postDelayMs);
		/** @brief Configure RS485 us-level pre/post TX delays. */
		/** @ingroup rs485_control */
		void setTxEnableDelaysUs(word preDelayUs, word postDelayUs);
		/** @brief Register read callback for specific address. */
		bool onRead(word address, modbusReadCallback cb);
		/** @brief Register write callback for specific address. */
		bool onWrite(word address, modbusWriteCallback cb);
		/** @brief Register callback for unsupported/custom function codes. */
		bool onUnknownFunction(modbusUnknownFunctionCallback cb);
		/** @brief Set 32-bit helper endianness mode on bound register bank. */
		void configureEndianness(byte mode);
		/** @brief Get active 32-bit helper endianness mode. */
		inline byte getEndianness(void) { return _device ? _device->getEndianness() : MODBUS_BIG_ENDIAN; }
		/** @brief Get FC08 bus message counter. */
		word getBusMessageCount(void);
		/** @brief Get FC08 bus communication error counter. */
		word getBusCommunicationErrorCount(void);
		/** @brief Get FC08 slave message counter. */
		word getSlaveMessageCount(void);
		/** @brief Reset all FC08 diagnostic counters to zero. */
		void clearDiagnosticsCounters(void);
		/** @brief Configure RTU baud rate and frame timing. */
		void setBaud(word);
		/** @brief Get current configured baud rate. */
		inline word getBaud(void) { return _baud; }
		/** @brief Calculate CRC for current frame buffer. */
		void calcCrc(void);
		/** @brief Legacy helper: check serial frame status. */
		void checkSerial(void);
		/** @brief Legacy helper: receive serial payload. */
		void serialRx(void);
		/** @brief Build digital status response payload. */
		void getDigitalStatus(byte, word, word);
		/** @brief Build analog status response payload. */
		void getAnalogStatus(byte, word, word);
		/** @brief Handle single-write function payloads. */
		void setStatus(byte, word, word);
		/** @brief Handle multi-write function payloads. */
		void setStatusMultiple(byte, word, word);
		/** @brief Handle FC08 diagnostics subfunctions. */
		void diagnostics(word, word);
		/**
		 * @brief Non-blocking slave engine tick.
		 * Call this continuously from Arduino loop().
		 */
		void run(void);

	private:
		bool isSupportedFunction(byte funcType);
		byte validateRequest(byte funcType, word field1, word field2, byte requestLen);
		bool validateRange(word startAddr, word count) const;
		modbusReadCallback findReadCallback(word address) const;
		modbusWriteCallback findWriteCallback(word address) const;
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
		bool _rxOverflow;
		unsigned long _lastRxByteUs;
};
/** @} */
#endif
