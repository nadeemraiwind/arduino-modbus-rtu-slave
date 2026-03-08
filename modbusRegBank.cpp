#include <modbusRegBank.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

modbusRegBank::modbusRegBank(void)
	: _digRegs(0),
	  _lastDigReg(0),
	  _anaRegs(0),
	  _lastAnaReg(0),
	  _endianness(MODBUS_BIG_ENDIAN),
	  _atomicLock(false),
	  _atomicDepth(0)
{
#if MODBUS_USE_STATIC_REG_POOL
	_digRegCount	= 0;
	_anaRegCount	= 0;
#endif
}


void modbusRegBank::add(word addr)
{
	if(this->has(addr))
		return;

	if(addr<20000)
	{
		modbusDigReg *temp;

#if MODBUS_USE_STATIC_REG_POOL
		if(_digRegCount >= MODBUS_MAX_DIG_REGS)
			return;
		temp = &_digRegPool[_digRegCount++];
#else
		temp = (modbusDigReg *) malloc(sizeof(modbusDigReg));
		if(temp == 0)
			return;
#endif
		temp->address = addr;
		temp->value		= 0;
		temp->next		= 0;

		if(_digRegs == 0)
		{
			_digRegs = temp;
			_lastDigReg = _digRegs;
		}
		else
		{
			//Assign the last register's next pointer to temp;
			_lastDigReg->next = temp;
			//then make temp the last register in the list.
			_lastDigReg = temp;
		}
	}	
	else
	{
		modbusAnaReg *temp;

#if MODBUS_USE_STATIC_REG_POOL
		if(_anaRegCount >= MODBUS_MAX_ANA_REGS)
			return;
		temp = &_anaRegPool[_anaRegCount++];
#else
		temp = (modbusAnaReg *) malloc(sizeof(modbusAnaReg));
		if(temp == 0)
			return;
#endif
		temp->address = addr;
		temp->value = 0;
		temp->next = 0;

		if(_anaRegs == 0)
		{
			_anaRegs = temp;
			_lastAnaReg = _anaRegs;
		}
		else
		{
			_lastAnaReg->next = temp;
			_lastAnaReg = temp;
		}
	}
}

word modbusRegBank::get(word addr)
{
	if(addr < 20000)
	{
		modbusDigReg * regPtr;
		regPtr = (modbusDigReg *) this->search(addr);
		if(regPtr)
			return(regPtr->value);
		else
			return(MODBUS_REG_NOT_FOUND);
	}
	else
	{
		modbusAnaReg * regPtr;
		regPtr = (modbusAnaReg *) this->search(addr);
		if(regPtr)
			return(regPtr->value);
		else
			return(MODBUS_REG_NOT_FOUND);
	}
}

void modbusRegBank::set(word addr, word value)
{
	//for digital data
	if(addr < 20000)
	{
		modbusDigReg * regPtr;
		//search for the register address
		regPtr = (modbusDigReg *) this->search(addr);
		//if a pointer was returned the set the register value to true if value is non zero
		if(regPtr)
			if(value)
				regPtr->value = 0xFF;
			else
				regPtr->value = 0x00;
	}
	else
	{
		modbusAnaReg * regPtr;
		//search for the register address
		regPtr = (modbusAnaReg *) this->search(addr);
		//if found then assign the register value to the new value.
		if(regPtr)
			regPtr->value = value;
	}
}

void modbusRegBank::atomicBegin(void)
{
	_atomicDepth++;
	_atomicLock = true;
}

void modbusRegBank::atomicEnd(void)
{
	if(_atomicDepth == 0)
		return;

	_atomicDepth--;
	if(_atomicDepth == 0)
		_atomicLock = false;
}

bool modbusRegBank::isAtomicLocked(void) const
{
	return _atomicLock;
}

word modbusRegBank::atomicGet(word addr)
{
	noInterrupts();
	word value = this->get(addr);
	interrupts();
	return value;
}

void modbusRegBank::atomicSet(word addr, word value)
{
	noInterrupts();
	this->set(addr, value);
	interrupts();
}

float modbusRegBank::getFloat(word addr)
{
	if(!this->has(addr) || !this->has(addr + 1))
		return NAN;

	union
	{
		float f;
		uint32_t u;
	} cvt;

	word hi;
	word lo;

	if((_endianness == MODBUS_LITTLE_ENDIAN) || (_endianness == MODBUS_BIG_ENDIAN_SWAPPED))
	{
		hi = this->get(addr + 1);
		lo = this->get(addr);
	}
	else
	{
		hi = this->get(addr);
		lo = this->get(addr + 1);
	}

	cvt.u = ((uint32_t)hi << 16) | lo;
	return cvt.f;
}

void modbusRegBank::setFloat(word addr, float value)
{
	if(!this->has(addr) || !this->has(addr + 1))
		return;

	union
	{
		float f;
		uint32_t u;
	} cvt;

	cvt.f = value;
	word hi = (word)(cvt.u >> 16);
	word lo = (word)(cvt.u & 0xFFFF);

	if((_endianness == MODBUS_LITTLE_ENDIAN) || (_endianness == MODBUS_BIG_ENDIAN_SWAPPED))
	{
		this->set(addr, lo);
		this->set(addr + 1, hi);
	}
	else
	{
		this->set(addr, hi);
		this->set(addr + 1, lo);
	}
}

uint32_t modbusRegBank::getLong(word addr)
{
	if(!this->has(addr) || !this->has(addr + 1))
		return 0UL;

	word hi;
	word lo;

	if((_endianness == MODBUS_LITTLE_ENDIAN) || (_endianness == MODBUS_BIG_ENDIAN_SWAPPED))
	{
		hi = this->get(addr + 1);
		lo = this->get(addr);
	}
	else
	{
		hi = this->get(addr);
		lo = this->get(addr + 1);
	}

	return ((uint32_t)hi << 16) | lo;
}

void modbusRegBank::setLong(word addr, uint32_t value)
{
	if(!this->has(addr) || !this->has(addr + 1))
		return;

	word hi = (word)(value >> 16);
	word lo = (word)(value & 0xFFFF);

	if((_endianness == MODBUS_LITTLE_ENDIAN) || (_endianness == MODBUS_BIG_ENDIAN_SWAPPED))
	{
		this->set(addr, lo);
		this->set(addr + 1, hi);
	}
	else
	{
		this->set(addr, hi);
		this->set(addr + 1, lo);
	}
}

word modbusRegBank::setString(word addr, const char *str, word maxRegs)
{
	word written = 0;
	word index = 0;

	if(str == 0)
		return 0;

	// Calculate string length once for efficiency
	word strLen = strlen(str);
	
	while(true)
	{
		if((maxRegs != 0) && (written >= maxRegs))
			break;

		word regAddr = addr + written;
		if(!this->has(regAddr))
			break;

		// Check bounds before accessing string
		byte c1 = (index < strLen) ? (byte)str[index] : 0;
		byte c2 = ((index + 1) < strLen) ? (byte)str[index + 1] : 0;
		
		word packed = ((word)c1 << 8) | c2;
		this->set(regAddr, packed);
		written++;

		if(c1 == 0 || c2 == 0)
			break;

		index += 2;
	}

	return written;
}

word modbusRegBank::getString(word addr, char *out, word outSize, word regCount)
{
	word copied = 0;

	if((out == 0) || (outSize == 0))
		return 0;

	for(word i = 0; i < regCount; i++)
	{
		word regAddr = addr + i;
		if(!this->has(regAddr))
			break;

		word value = this->get(regAddr);
		byte c1 = (byte)(value >> 8);
		byte c2 = (byte)(value & 0xFF);

		if((copied + 1) >= outSize)
			break;

		if(c1 == 0)
			break;
		out[copied++] = (char)c1;

		if((copied + 1) >= outSize)
			break;

		if(c2 == 0)
			break;
		out[copied++] = (char)c2;
	}

	out[copied] = '\0';
	return copied;
}

void modbusRegBank::setEndianness(byte mode)
{
	if((mode == MODBUS_BIG_ENDIAN) ||
	   (mode == MODBUS_LITTLE_ENDIAN) ||
	   (mode == MODBUS_BIG_ENDIAN_SWAPPED))
		_endianness = mode;
}

byte modbusRegBank::getEndianness(void)
{
	return _endianness;
}

bool modbusRegBank::has(word addr)
{
	return (this->search(addr) != 0);
}

void * modbusRegBank::search(word addr)
{
	//if the requested address is 0-19999 
	//use a digital register pointer assigned to the first digital register
	//else use a analog register pointer assigned the first analog register

	if(addr < 20000)
	{
		modbusDigReg *regPtr = _digRegs;

		//if there is no register configured, bail
		if(regPtr == 0)
			return(0);

		//scan through the linked list until the end of the list or the register is found.
		//return the pointer.
		do
		{
			if(regPtr->address == addr)
				return(regPtr);
			regPtr = regPtr->next;
		}
		while(regPtr);
	}
	else
	{
		modbusAnaReg *regPtr = _anaRegs;

		//if there is no register configured, bail
		if(regPtr == 0)
			return(0);

		//scan through the linked list until the end of the list or the register is found.
		//return the pointer.
		do
		{
			if(regPtr->address == addr)
				return(regPtr);
			regPtr = regPtr->next;
		}
		while(regPtr);
	}
	return(0);
}


