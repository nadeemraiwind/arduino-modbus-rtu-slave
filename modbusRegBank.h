#include <stdint.h>
#include <Arduino.h>

#ifndef _MODBUSREGBANK
#define _MODBUSREGBANK

#include <modbus.h>

/**
 * @file modbusRegBank.h
 * @brief Register storage engine and typed data helpers for Modbus maps.
 */

/** @defgroup data_access_helpers Data Access Helpers
 * Register storage, typed conversion helpers, and addressing utilities.
 * @{
 */

/**
 * @brief Linked-node representation for digital register domains.
 * 
 * Memory layout: 6 bytes on AVR (2+1+2+padding), may differ on 32-bit platforms.
 * Structure is naturally aligned for performance. For extreme RAM optimization on
 * ARM/ESP32, consider __attribute__((packed)), but this typically degrades performance.
 */
struct modbusDigReg
{
	/** @brief Modicon-style address. */
	word address;
	/** @brief Digital payload. */
	byte value;

	/** @brief Next list node. */
	modbusDigReg *next;
};

/**
 * @brief Linked-node representation for analog register domains.
 * 
 * Memory layout: 6 bytes on AVR (2+2+2), 8 bytes on 32-bit platforms (due to pointer size).
 */
struct modbusAnaReg
{
	/** @brief Modicon-style address. */
	word address;
	/** @brief 16-bit payload. */
	word value;

	/** @brief Next list node. */
	modbusAnaReg *next;
};

/**
 * @class modbusRegBank
 * @brief Register collection with 16-bit and typed (32-bit/string) accessors.
 * @ingroup data_access_helpers
 */
class modbusRegBank
{
	public:

		/** @brief Create an empty register bank. */
		modbusRegBank(void);
		
		/** @brief Add a register address to the map. */
		void add(word address);
		/** @brief Get register value or @ref MODBUS_REG_NOT_FOUND when missing. */
		word get(word address);
		/** @brief Set register value. */
		void set(word address, word value);
		/**
		 * @brief Begin an application-level atomic transaction.
		 *
		 * While active, @ref modbusSlave::run can defer frame processing when
		 * using this register bank through modbusDevice.
		 */
		void atomicBegin(void);
		/** @brief End an application-level atomic transaction. */
		void atomicEnd(void);
		/** @brief Returns true when an atomic transaction is active. */
		bool isAtomicLocked(void) const;
		/** @brief Atomic wrapper around get(). */
		word atomicGet(word address);
		/** @brief Atomic wrapper around set(). */
		void atomicSet(word address, word value);
		/** @brief Read 32-bit float from two adjacent registers. */
		float getFloat(word address);
		/** @brief Write 32-bit float to two adjacent registers. */
		void setFloat(word address, float value);
		/** @brief Read 32-bit integer from two adjacent registers. */
		uint32_t getLong(word address);
		/** @brief Write 32-bit integer to two adjacent registers. */
		void setLong(word address, uint32_t value);
		/**
		 * @brief Write ASCII string (2 chars/register) starting at @p address.
		 * @return Number of registers written.
		 */
		word setString(word address, const char *str, word maxRegs = 0);
		/**
		 * @brief Read ASCII string from packed register area.
		 * @param address Starting register address (Modicon style).
		 * @param out Output buffer to receive null-terminated string.
		 * @param outSize Maximum size of output buffer (including null terminator).
		 * @param regCount Number of registers to read from (2 chars per register).
		 * @return Number of characters written to output buffer (excluding null terminator).
		 */
		word getString(word address, char *out, word outSize, word regCount);
		/**
		 * @brief Set helper endianness mode for 32-bit encode/decode helpers.
		 * @param mode Endianness mode: MODBUS_BIG_ENDIAN (default), MODBUS_LITTLE_ENDIAN, or MODBUS_BIG_ENDIAN_SWAPPED.
		 */
		void setEndianness(byte mode);
		/** @brief Get current helper endianness mode. */
		byte getEndianness(void);
		/** @brief Check whether register exists. */
		bool has(word address);
				
	private:
		void * search(word address);
		
		modbusDigReg	*_digRegs,
						*_lastDigReg;
							
		modbusAnaReg	*_anaRegs,
						*_lastAnaReg;
		byte			_endianness;
		volatile bool	_atomicLock;
		byte			_atomicDepth;

#if MODBUS_USE_STATIC_REG_POOL
		modbusDigReg	_digRegPool[MODBUS_MAX_DIG_REGS];
		modbusAnaReg	_anaRegPool[MODBUS_MAX_ANA_REGS];
		word			_digRegCount;
		word			_anaRegCount;
#endif
};
/** @} */
#endif
