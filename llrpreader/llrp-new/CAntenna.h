#ifndef __CANTENNA_H__
#define __CANTENNA_H__

#include "OwTask.h"

#define MAX_ANT_CNT			256
#define ANT_DEFAULT_POWER	2500		//25dBm

class CAntenna
{
public:
 
	CAntenna(){}
    ~CAntenna(){}

    static int	  m_antcount;
    static int	  m_antlist[MAX_ANT_CNT];
    static UINT16 m_antpower[MAX_ANT_CNT];
	static UINT16 m_wrpower[MAX_ANT_CNT];
	
	static void Set_TxPower(int antid, int pwr);
	static void Set_WrPower(int antid, int pwr);
	static void Get_TxPower(int antid, int *pwr);
	static void Get_WrPower(int antid, int *pwr);
	static void Set_TxPower_Default(void);
	static void Set_WrPower_Default(void);
};

#endif
