#include <stdint.h>
#ifndef _MODBUSPROTOCOL
#define _MODBUSPROTOCOL

//Maximum device list for network
#define DEVMAX		10
//Maximum control register que size
#define QUEMAX		10
//Maximum serial wait in micro seconds
#define SERIALMAXDELAY	100
#define SERIALBAUD		9600
#define MODBUS_MAX_FRAME	128
#define MODBUS_REG_NOT_FOUND 0xFFFF

// Optional memory model: use static pools instead of malloc-backed linked nodes.
// Set to 1 for long-running deployments where heap fragmentation is a concern.
#ifndef MODBUS_USE_STATIC_REG_POOL
#define MODBUS_USE_STATIC_REG_POOL 0
#endif

#ifndef MODBUS_MAX_DIG_REGS
#define MODBUS_MAX_DIG_REGS 64
#endif

#ifndef MODBUS_MAX_ANA_REGS
#define MODBUS_MAX_ANA_REGS 64
#endif
//the total silence time needed to signify an EOM or SOM in RTU mode

//Modbus function codes
#define READ_DO 	0x01
#define READ_DI		0x02
#define READ_AO 	0x03
#define READ_AI		0x04

#define WRITE_DO	0x05
#define WRITE_AO	0x06
#define WRITE_DO_MULTI 0x0F
#define WRITE_AO_MULTI 0x10
#define DIAGNOSTICS 0x08

// 32-bit data word-order configuration for getFloat/getLong helpers.
#define MODBUS_BIG_ENDIAN 0
#define MODBUS_LITTLE_ENDIAN 1
#define MODBUS_BIG_ENDIAN_SWAPPED 2

// FC08 diagnostics subfunctions implemented by this library.
#define DIAG_RETURN_QUERY_DATA 0x0000
#define DIAG_BUS_MESSAGE_COUNT 0x000B
#define DIAG_BUS_COMM_ERROR_COUNT 0x000C
#define DIAG_SLAVE_MESSAGE_COUNT 0x000E

// Callback hook capacities (per slave instance).
#ifndef MODBUS_MAX_READ_CALLBACKS
#define MODBUS_MAX_READ_CALLBACKS 8
#endif

#ifndef MODBUS_MAX_WRITE_CALLBACKS
#define MODBUS_MAX_WRITE_CALLBACKS 8
#endif

#ifndef MODBUS_MAX_UNKNOWN_FUNCTION_CALLBACKS
#define MODBUS_MAX_UNKNOWN_FUNCTION_CALLBACKS 1
#endif

#define RTU 		0x01
#define ASCII		0x02

#define MASTER		0x01
#define SLAVE		0x02

#define DO			0x00
#define DI			0x01
#define AI			0x03
#define AO			0x04

//Modbus exception codes
#define EX_ILLEGAL_FUNCTION		0x01
#define EX_ILLEGAL_DATA_ADDRESS	0x02
#define EX_ILLEGAL_DATA_VALUE		0x03

#endif