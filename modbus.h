#include <stdint.h>
#ifndef _MODBUSPROTOCOL
#define _MODBUSPROTOCOL

/**
 * @file modbus.h
 * @brief Core protocol constants, limits, and compile-time configuration for the Modbus RTU library.
 */

/** @defgroup protocol_engine Protocol Engine
 * Core protocol constants, function codes, diagnostics codes, and exception values.
 * @{
 */

/** @brief Maximum device list size for network-level arrays. */
#define DEVMAX		10
/** @brief Maximum control register queue size. */
#define QUEMAX		10
/** @brief Maximum serial wait in microseconds. */
#define SERIALMAXDELAY	100
/** @brief Default serial baud rate. */
#define SERIALBAUD		9600
/** @brief Maximum supported RTU frame size in bytes. */
#define MODBUS_MAX_FRAME	128
/** @brief Sentinel returned by get() when a register does not exist. */
#define MODBUS_REG_NOT_FOUND 0xFFFF

/**
 * @brief Optional memory model: use static pools instead of malloc-backed linked nodes.
 * Set to 1 for long-running deployments where heap fragmentation is a concern.
 */
#ifndef MODBUS_USE_STATIC_REG_POOL
#define MODBUS_USE_STATIC_REG_POOL 0
#endif

#ifndef MODBUS_MAX_DIG_REGS
#define MODBUS_MAX_DIG_REGS 64
#endif

#ifndef MODBUS_MAX_ANA_REGS
#define MODBUS_MAX_ANA_REGS 64
#endif
/** @name Modbus Function Codes */
/**@{*/
#define READ_DO 	0x01
#define READ_DI		0x02
#define READ_AO 	0x03
#define READ_AI		0x04

#define WRITE_DO	0x05
#define WRITE_AO	0x06
#define WRITE_DO_MULTI 0x0F
#define WRITE_AO_MULTI 0x10
#define DIAGNOSTICS 0x08
/**@}*/

/** @name 32-bit Endianness Modes for getFloat/getLong Helpers */
/**@{*/
#define MODBUS_BIG_ENDIAN 0
#define MODBUS_LITTLE_ENDIAN 1
#define MODBUS_BIG_ENDIAN_SWAPPED 2
/**@}*/

/** @name FC08 Diagnostics Subfunctions */
/**@{*/
#define DIAG_RETURN_QUERY_DATA 0x0000
#define DIAG_BUS_MESSAGE_COUNT 0x000B
#define DIAG_BUS_COMM_ERROR_COUNT 0x000C
#define DIAG_SLAVE_MESSAGE_COUNT 0x000E
/**@}*/

/**
 * @brief Callback hook capacities (per slave instance).
 * Increase these values if your project needs more callback-mapped addresses.
 */
#ifndef MODBUS_MAX_READ_CALLBACKS
#define MODBUS_MAX_READ_CALLBACKS 8
#endif

#ifndef MODBUS_MAX_WRITE_CALLBACKS
#define MODBUS_MAX_WRITE_CALLBACKS 8
#endif

#ifndef MODBUS_MAX_UNKNOWN_FUNCTION_CALLBACKS
#define MODBUS_MAX_UNKNOWN_FUNCTION_CALLBACKS 1
#endif

/** @brief RTU mode constant. */
#define RTU 		0x01
/** @brief ASCII mode constant (currently reserved/not implemented). */
#define ASCII		0x02

/** @brief Master role constant. */
#define MASTER		0x01
/** @brief Slave role constant. */
#define SLAVE		0x02

#define DO			0x00
#define DI			0x01
#define AI			0x03
#define AO			0x04

/** @name Modbus Exception Codes */
/**@{*/
#define EX_ILLEGAL_FUNCTION		0x01
#define EX_ILLEGAL_DATA_ADDRESS	0x02
#define EX_ILLEGAL_DATA_VALUE		0x03
/**@}*/

/** @} */
#endif