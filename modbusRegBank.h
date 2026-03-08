
#include <stdint.h>
#include <Arduino.h>

#ifndef _MODBUSREGBANK
#define _MODBUSREGBANK

#include <modbus.h>
//#include <Wprogram.h>


struct modbusDigReg
{
	word address;
	byte value;

	modbusDigReg *next;
};

struct modbusAnaReg
{
	word address;
	word value;

	modbusAnaReg *next;
};

class modbusRegBank
{
	public:

		modbusRegBank(void);
		
		void add(word);
		word get(word);
		void set(word, word);
		float getFloat(word);
		void setFloat(word, float);
		uint32_t getLong(word);
		void setLong(word, uint32_t);
		word setString(word, const char *, word maxRegs = 0);
		word getString(word, char *, word outSize, word regCount);
		void setEndianness(byte mode);
		byte getEndianness(void);
		bool has(word);
				
	private:
		void * search(word);
		
		modbusDigReg	*_digRegs,
						*_lastDigReg;
							
		modbusAnaReg	*_anaRegs,
						*_lastAnaReg;
		byte			_endianness;

#if MODBUS_USE_STATIC_REG_POOL
		modbusDigReg	_digRegPool[MODBUS_MAX_DIG_REGS];
		modbusAnaReg	_anaRegPool[MODBUS_MAX_ANA_REGS];
		word			_digRegCount;
		word			_anaRegCount;
#endif
};
#endif
