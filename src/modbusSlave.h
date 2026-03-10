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
 *
 * RTU timing reference used by @ref modbusSlave::setBaud "setBaud()":
 *
 * | Baud | Character Time (10 bits) | T1.5 Silent Interval | T3.5 Silent Interval |
 * | ---- | ------------------------ | -------------------- | -------------------- |
 * | 9600   | 1041.7 us | 1562.5 us | 3645.8 us |
 * | 19200  | 520.8 us  | 781.3 us  | 1822.9 us |
 * | 115200 | 86.8 us   | 130.2 us  | 303.8 us  |
 */

/** @defgroup callback_hooks Callback Hooks
 * Dynamic read/write behavior and custom function handlers.
 */

/** @brief CRC lookup high-byte table, defined in modbusSlave.cpp. */
extern const byte _auchCRCHi[256];
/** @brief CRC lookup low-byte table, defined in modbusSlave.cpp. */
extern const byte _auchCRCLo[256];

/** @brief Callback signature for dynamic read values. */
/** @ingroup callback_hooks */
typedef uint16_t (*modbusReadCallback)(word address, modbusDevice *device);
/** @brief Callback signature invoked after register writes. */
/** @ingroup callback_hooks */
typedef void (*modbusWriteCallback)(word address, word value, modbusDevice *device);
/** @brief Callback signature for custom/unknown function code handling. */
/** @ingroup callback_hooks */
typedef bool (*modbusUnknownFunctionCallback)(byte funcType, const byte *request, byte requestLen, byte *response, byte *responseLen, modbusDevice *device);
/** @brief Callback signature to reconfigure baud on non-HardwareSerial Stream transports. */
/** @ingroup callback_hooks */
typedef void (*modbusStreamBaudCallback)(word baud, Stream *port);

/**
 * @class modbusSlave
 * @brief Modbus RTU slave communications and frame processing engine.
 * @ingroup protocol_engine
 *
 * Internal parser state machine used by @ref modbusSlave::run "run()":
 *
 * @dot
 * digraph ParserFSM {
 *   rankdir=LR;
 *   node [shape=box, style=rounded, fontname=Helvetica, fontsize=10];
 *
 *   IDLE [label="PARSER_IDLE"];
 *   RX [label="PARSER_RECEIVING"];
 *   COMPLETE [label="PARSER_COMPLETE"];
 *
 *   IDLE -> RX [label="first byte received"];
 *   RX -> RX [label="more bytes;\nreset silence timer"];
 *   RX -> COMPLETE [label="silence >= T3.5"];
 *   RX -> IDLE [label="overflow or reset"];
 *   COMPLETE -> IDLE [label="processFrame() done"];
 * }
 * @enddot
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
		/** @ingroup callback_hooks */
		/** @see 04_Expert_Callbacks.ino */
		bool onRead(word address, modbusReadCallback cb);
		/** @brief Register write callback for specific address. */
		/** @ingroup callback_hooks */
		bool onWrite(word address, modbusWriteCallback cb);
		/** @brief Register callback for unsupported/custom function codes. */
		/** @ingroup callback_hooks */
		bool onUnknownFunction(modbusUnknownFunctionCallback cb);
		/** @brief Register baud reconfiguration callback for generic Stream transports. */
		/** @ingroup callback_hooks */
		bool setStreamBaudHandler(modbusStreamBaudCallback cb);
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
		/**
		 * @brief Reset all FC08 diagnostic counters to zero.
		 * Useful for periodic diagnostics reset or after maintenance.
		 */
		void clearDiagnosticsCounters(void);
		/** @brief Configure RTU baud rate and frame timing. */
		void setBaud(word);
		/** @brief Select protocol transport framing mode (RTU default, optional ASCII). */
		void setProtocol(byte mode);
		/** @brief Get active protocol framing mode. */
		inline byte getProtocol(void) { return _protocol; }
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
		/** @brief Check if function code is natively supported. */
		bool isSupportedFunction(byte funcType);
		/** @brief Validate request frame structure and data ranges. */
		byte validateRequest(byte funcType, word field1, word field2, byte requestLen);
		/** @brief Verify all registers in range exist in device model. */
		bool validateRange(word startAddr, word count) const;
		/** @brief Search for read callback by address. */
		modbusReadCallback findReadCallback(word address) const;
		/** @brief Search for write callback by address. */
		modbusWriteCallback findWriteCallback(word address) const;
		/** @brief Execute read callback if registered, else return fallback. */
		word invokeReadCallback(word address, word fallbackValue);
		/** @brief Execute write callback if registered. */
		void invokeWriteCallback(word address, word value);
		/** @brief Reset parser state to idle. */
		void resetParser(void);
		/** @brief Return expected RTU frame length once determinable from buffered bytes. */
		byte expectedFrameLength(void) const;
		/** @brief Decode collected ASCII frame into internal binary ADU buffer. */
		bool decodeAsciiFrame(void);
		/** @brief Process complete frame and generate response. */
		void processFrame(void);
		/**
		 * @brief Build and send Modbus exception response.
		 *
		 * Exception trigger reference:
		 *
		 * | Code | Name | Typical Trigger in This Library |
		 * | ---- | ---- | ------------------------------- |
		 * | 0x01 | Illegal Function | Unsupported function code, or onUnknownFunction callback returned false. |
		 * | 0x02 | Illegal Data Address | Requested register/range does not exist in modbusDevice/modbusRegBank. |
		 * | 0x03 | Illegal Data Value | Request quantity/field is invalid (for example exceeds frame constraints for multi-write). |
		 */
		void sendException(byte funcType, byte exceptionCode);
		/** @brief Transmit frame buffer with RS485 direction control. */
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
		byte _protocol;


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
		modbusStreamBaudCallback _streamBaudCallback;
		ParserState _parserState;
		byte _rxIndex;
		byte _asciiIndex;
		bool _rxOverflow;
		bool _asciiInFrame;
		char _asciiBuf[(MODBUS_MAX_FRAME * 2) + 8];
		unsigned long _lastRxByteUs;
};
#endif
