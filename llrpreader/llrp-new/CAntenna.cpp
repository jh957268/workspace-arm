#include "CAntenna.h"

int CAntenna::m_antcount = 0;
int    CAntenna::m_antlist[] = {0};
UINT16 CAntenna::m_antpower[] = {0};
UINT16 CAntenna::m_wrpower[] = {0};


void CAntenna::Set_TxPower(int antid, int pwr)
{
	m_antpower[antid - 1] = (UINT16)pwr;
}

void CAntenna::Set_WrPower(int antid, int pwr)
{
	m_wrpower[antid - 1] = (UINT16)pwr;
}

void CAntenna::Get_TxPower(int antid, int *pwr)
{
	*pwr = m_antpower[antid - 1];
}

void CAntenna::Get_WrPower(int antid, int *pwr)
{
	*pwr = m_wrpower[antid - 1];;
}

void CAntenna::Set_TxPower_Default(void)
{
	for (int i = 0; i < MAX_ANT_CNT; i++)
	{
		m_antpower[i] = ANT_DEFAULT_POWER;
	}
}

void CAntenna::Set_WrPower_Default(void)
{
	for (int i = 0; i < MAX_ANT_CNT; i++)
	{
		m_antpower[i] = ANT_DEFAULT_POWER;
	}
}
