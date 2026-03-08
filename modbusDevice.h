#include <stdint.h>
#include <Arduino.h>

#ifndef _MODBUSDEVICE
#define _MODBUSDEVICE

#include <modbusRegBank.h>
#include <modbus.h>
//#include <Wprogram.h>

/**
 * @file modbusDevice.h
 * @brief Modbus device abstraction that extends the register bank with slave ID handling.
 */

/**
 * @class modbusDevice
 * @brief Represents a Modbus slave device data model.
 *
 * Inherits register storage/access behavior from @ref modbusRegBank and adds
 * the device/slave address used by @ref modbusSlave when validating frames.
 */
class modbusDevice:public modbusRegBank
{
	public:
		/** @brief Constructs a device instance with default settings. */
		modbusDevice(void);
		/**
		 * @brief Sets the Modbus slave ID.
		 * @param id Slave address (typically 1..247).
		 */
		void setId(byte id);
		/**
		 * @brief Gets the current Modbus slave ID.
		 * @return Configured slave ID.
		 */
		byte getId(void);
		
	private:
		byte _id;
};
#endif
