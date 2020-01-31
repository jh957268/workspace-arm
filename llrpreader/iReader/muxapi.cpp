//#include "conio.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 

#include <sys/stat.h> 

#include <fcntl.h> 
#include "define.h"
#include "muxclient.h"
#include "muxserial.h"
#include "muxerror.h"

#define __stdcall

#define MAX_MUX_DEVICES		128
#define MAX_COM_PORTS		5

static void *muxDevHandle[MAX_MUX_DEVICES];
static int muxDevInit = 0;

char *MuxretBuff;

static const char *comPort [] =
{
	"COM1",
	"COM2",
	"COM3",
	"COM4",
	"COM5"
};

UInt32 __stdcall MuxApiDevCreate(char *remote)
{
	int i;
	Muxdev *devHandle;

		if (muxDevInit == 0)
	{
		for (i = 0; i < MAX_MUX_DEVICES; i++)
			muxDevHandle[i] = (void *)NULL;
		muxDevInit = 1;
	}
	for (i = 0; i < MAX_COM_PORTS; i++)
	{
		if (!strcmp(remote, comPort[i]))
			break; 		
	}
	if (i < MAX_COM_PORTS)
	{
		devHandle = new Muxserial(remote);
	}
	else
	{
		devHandle = new MuxClient(remote);
	}
	if (devHandle != NULL)
	{
		for (i = 0; i < MAX_MUX_DEVICES; i++)
		{
			if (muxDevHandle[i] == NULL)
				break;
		}
		if (i >= MAX_MUX_DEVICES)
		{
			free(devHandle);
			devHandle = NULL;
		}
		else
		{
			muxDevHandle[i] = (void *)devHandle;
		}
	}
	return((UInt32)devHandle); 
}

Int32 MuxHandleValid(UInt32 handle)
{
	int i;
	int error = MUX_SUCCESS;

	if (muxDevInit == 0)
	{
		return (MUX_NOT_INTIALIZED);
	}
	for (i = 0; i < MAX_MUX_DEVICES; i++)
	{
		if (muxDevHandle[i]	== 	(void *)handle)
			break; 		
	}
	if (i >= MAX_MUX_DEVICES)
	{
		error = MUX_HANDLE_INVALID;
	}
	return (error);
}

Int32 MuxHandleEmptySlot(UInt32 handle)
{
	int i;
	int error = MUX_SUCCESS;

	if (muxDevInit == 0)
	{
		return (MUX_NOT_INTIALIZED);
	}
	for (i = 0; i < MAX_MUX_DEVICES; i++)
	{
		if (muxDevHandle[i]	== 	(void *)handle)
		{
			muxDevHandle[i]	= (void *)NULL;
			break;
		} 		
	}
	if (i >= MAX_MUX_DEVICES)
	{
		error = MUX_HANDLE_INVALID;
	}
	return (error);
}

Int32 __stdcall MuxApiDevConnect(UInt32 handle)
{
	int error;
	Muxdev *devHandle;

	error = MuxHandleValid(handle);
	if (error == MUX_SUCCESS)
	{
		devHandle = (Muxdev *)handle;
		error = devHandle->Connect();
	}
	return(error); 
}

Int32 __stdcall MuxApiDevSetport(UInt32 handle, Int32 port)
{
	Int32 error;
	Muxdev *devHandle;

	if (port < 1 || port > 256)
		return(MUX_INVALID); 
	error = MuxHandleValid(handle);
	if (error == MUX_SUCCESS)
	{
		devHandle = (Muxdev *)handle;
		error = devHandle->SetAntPort(port-1);
		
	}
	return(error); 
}

Int32 __stdcall MuxApiDevGetMap(UInt32 handle, uint8_t *map)
{
	int error;
	Muxdev *devHandle;

	error = MuxHandleValid(handle);
	if (error == MUX_SUCCESS)
	{
		devHandle = (Muxdev *)handle;
		error = devHandle->GetAntMap(map);
		
	}
	return(error); 
}

Int32 __stdcall MuxApiDevClose(UInt32 handle)
{
	int error;
	Muxdev *devHandle;

	error = MuxHandleValid(handle);
	if (error == MUX_SUCCESS)
	{
		devHandle = (Muxdev *)handle;
		devHandle->Disconnect();
		MuxHandleEmptySlot(handle);
		free((void *)handle);
	}
	return(error); 
}

Int32 __stdcall MuxApiDevRescanSlave(UInt32 handle, Int32 chn)
{
	int error;
	Muxdev *devHandle;

	if (chn < 1 || chn > 8)
		return(MUX_INVALID); 

	error = MuxHandleValid(handle);
	if (error == MUX_SUCCESS)
	{
		devHandle = (Muxdev *)handle;
		error = devHandle->RescanSlave(chn-1);
		
	}
	return(error); 
}

Int32 __stdcall MuxApiDevGetMasterVer(UInt32 handle, Int32 & ver)
{
	int error;
	Muxdev *devHandle;
	Int32 version;

	error = MuxHandleValid(handle);
	if (error == MUX_SUCCESS)
	{
		devHandle = (Muxdev *)handle;
		error = devHandle->GetMasterSoftVer(&version);
		ver = version;
		
	}
	return(error); 
}

Int32 __stdcall MuxApiDevGetSlaveVer(UInt32 handle, Int32 chn, Int32 slave, Int32 & ver)
{
	int error;
	Muxdev *devHandle;
	Int32 version;

	if (chn < 1 || chn > 8 || slave < 1 || slave > 4)
		return(MUX_INVALID); 

	error = MuxHandleValid(handle);
	if (error == MUX_SUCCESS)
	{
		devHandle = (Muxdev *)handle;
		error = devHandle->GetSlaveSoftVer(chn-1, slave-1, &version);
		ver = version;
	}
	return(error); 
}

Int32 __stdcall MuxApiDevGetSlaveStats(UInt32 handle, uint8_t *stats)
{
	int error;
	Muxdev *devHandle;

	error = MuxHandleValid(handle);
	if (error == MUX_SUCCESS)
	{
		devHandle = (Muxdev *)handle;
		error = devHandle->GetSlaveStat(stats);
		
	}
	return(error); 
}

Int32 __stdcall MuxApiDevSelectModule(UInt32 handle, E_MODULE module)
{
	int error;
	MuxClient *devHandle;
	//Int32 version;

	error = MuxHandleValid(handle);
	if (error == MUX_SUCCESS)
	{
		devHandle = (MuxClient *)handle;
		error = devHandle->SelectModule(module);
		
	}
	return(error); 
}

Int32 __stdcall MuxApiDevGetChar(UInt32 handle, unsigned char *ch)
{
	int error;
	Muxdev *devHandle;

	error = MuxHandleValid(handle);
	if (error == MUX_SUCCESS)
	{
		devHandle = (Muxdev *)handle;
		error = devHandle->getChar(ch);
		
	}
	return(error); 
}

Int32 __stdcall MuxApiDevSendChar(UInt32 handle, unsigned char ch)
{
	int error;
	Muxdev *devHandle;

	error = MuxHandleValid(handle);
	if (error == MUX_SUCCESS)
	{
		devHandle = (Muxdev *)handle;
		error = devHandle->sendChar(ch);
		
	}
	return(error); 
}
