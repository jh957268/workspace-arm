//#include "stdafx.h"
#include "HostTypes.h"
#include "CrcUtils.h"

//#ifdef  __cplusplus
//extern "C" {
//#endif


/** @fn void CRC_calcCrc8(u16 *crcReg, u16 poly, u16 u8Data)
  * @brief Standard CRC calculation on an 8-bit piece of data.  To make it
  *					CCITT-16, use poly=0x1021 and an initial crcReg=0xFFFF.
  * 
  * 				Note:  This function allows one to call it repeatedly to continue
  *									calculating a CRC.  Thus, the first time it's called, it
  *									should have an initial crcReg of 0xFFFF, after which it
  *									can be called with its own result.
  *
  * @param *crcReg	Pointer to current CRC register.
  * @param poly			Polynomial to apply.
  * @param u8Data		u8 data to perform CRC on.
  * @return None.
  */
void CRC_calcCrc8(u16 *crcReg, u16 poly, u16 u8Data)
{
	u16 i;
	u16 xorFlag;
	u16 bit;
	u16 dcdBitMask = 0x80;

	for(i=0; i<8; i++)
	{
		// Get the carry bit.  This determines if the polynomial should be xor'd
		//	with the CRC register.
		xorFlag = *crcReg & 0x8000;
		
		// Shift the bits over by one.
		*crcReg <<= 1;
		
		// Shift in the next bit in the data byte
		bit = ((u8Data & dcdBitMask) == dcdBitMask);
		*crcReg |= bit;
		
		// XOR the polynomial
		if(xorFlag)
		{
			*crcReg = *crcReg ^ poly;
		}
		
		// Shift over the dcd mask
		dcdBitMask >>= 1;	
	}
}


//#ifdef  __cplusplus
//}
//#endif

