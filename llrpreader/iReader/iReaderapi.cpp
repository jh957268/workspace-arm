//#include "conio.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 

#include <sys/stat.h> 

#include <fcntl.h> 
#include "define.h"
#include "iReadererror.h"
#include "iReader.h"
#include "muxserial.h"
#include "iReaderapi.h"

#define __cdecl

#define MAX_IREADER_DEVICES		1

#define	IF_ERROR_RETURN(funcall) do {				    \
	    int errcode;					                \
	    if ((errcode = (funcall)) != 0)					\
        {                                               \
            if ( (IREADER_NOT_INTIALIZED != errcode) && (IREADER_HANDLE_INVALID != errcode)) \
            {                                           \
	            ((IReader *)handle)->IReaderGiveMutex(); \
            }                                           \
	        return(errcode);                            \
        }                                               \
    } while (0)

static void *iReaderHandle[MAX_IREADER_DEVICES];
static int iReaderInit = 0;


Int32 IReaderHandleValid(void * handle)
{
	int i;
	int error = IREADER_SUCCESS;


	if (iReaderInit == 0)
	{
		return (IREADER_NOT_INTIALIZED);
	}
	for (i = 0; i < MAX_IREADER_DEVICES; i++)
	{
		if (iReaderHandle[i] == (void *)handle)
			break; 		
	}
	if (i >= MAX_IREADER_DEVICES)
	{
		error = IREADER_HANDLE_INVALID;
	}
	return (error);
}

Int32 IReaderHandleEmptySlot(void * handle)
{
	int i;
	int error = IREADER_SUCCESS;

	if (iReaderInit == 0)
	{
		return (IREADER_NOT_INTIALIZED);
	}
	for (i = 0; i < MAX_IREADER_DEVICES; i++)
	{
		if (iReaderHandle[i] == (void *)handle)
		{
			iReaderHandle[i] = (void *)NULL;
			break;
		} 		
	}
	if (i >= MAX_IREADER_DEVICES)
	{
		error = IREADER_HANDLE_INVALID;
	}
	return (error);
}

void * __cdecl IReaderApiCreate(char *remote)
{
	int i;
	IReader *devHandle;

	if (iReaderInit == 0)
	{
		for (i = 0; i < MAX_IREADER_DEVICES; i++)
			iReaderHandle[i] = (void *)NULL;
		iReaderInit = 1;
	}
    
	devHandle = IReader::getInstance();

	if (devHandle != NULL)
	{
		for (i = 0; i < MAX_IREADER_DEVICES; i++)
		{
			if (iReaderHandle[i] == NULL)
				break;
		}
		if (i >= MAX_IREADER_DEVICES)
		{
			delete(devHandle);
			devHandle = NULL;
		}
		else
		{
			iReaderHandle[i] = (void *)devHandle;
		}
	}

	return((void *)devHandle); 
}

void * __cdecl IReaderApiInit(char *remote, int region)
{
	int i;
	IReader *devHandle;
	int error = IREADER_SUCCESS;

    devHandle = (IReader *)IReaderApiCreate(remote);
    if (NULL == devHandle)
    {
	    return((void *)NULL); 
    }
    
	error = devHandle->IReaderInit(remote, region);

    for (i = 0; i < 256; i++)
	    devHandle->IReaderSetPowerLevel(i, DEFAULT_TX_POWER, 0);  // setup dedfault power level 25dBm
	devHandle->IReaderTagSearchTimeout(DEFAULT_TAG_SEARCH_TIME);     // default 80ms
    
	// devHandle->IReaderGetAntMap();      //  setup the antenna map at iReader object 
	devHandle->IReaderCreateMutex(); 
    
	return((void *)devHandle); 
}

Int32 __cdecl IReaderApiConnect(void * handle)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderConnect();
	}
	return(error); 
}

Int32 __cdecl IReaderApiGetAntMap(void * handle)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderGetAntMap();
		
	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error); 
}

Int32 __cdecl IReaderApiGetAntList(void * handle, int *antCount, int *antList)
{
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();
    IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(MUX_MODULE));
    IF_ERROR_RETURN(((IReader *)handle)->IReaderGetAntList(antCount, antList));
    IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(READER_MODULE));
	((IReader *)handle)->IReaderGiveMutex();
    return IREADER_SUCCESS;

#if 0
	error = IReaderHandleValid(handle);
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderGetAntList(antCount, antList);
		
	}
	return(error); 
#endif
}


Int32 __cdecl IReaderApiClose(void * handle)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		devHandle->IReaderDisconnect();
		IReaderHandleEmptySlot(handle);
		devHandle->IReaderCloseMutex();
		free((void *)handle);
		// devHandle->IReaderCloseMutex();
	}
	return(error); 
}

Int32 __cdecl IReaderApiScanSlave(void * handle, Int32 chn)
{

	if (chn < 1 || chn > 8)
		return(IREADER_INVALID); 

    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();
    IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(MUX_MODULE));
    IF_ERROR_RETURN(((IReader *)handle)->IReaderRescanSlave(chn - 1));
    IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(READER_MODULE));
	((IReader *)handle)->IReaderGiveMutex();
    return IREADER_SUCCESS;
}

Int32 __cdecl IReaderApiSetRegion(void * handle, int region)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderSetRegion(region);
		
	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error); 
}

Int32 __cdecl IReaderApiSyncChannel(void * handle, int channel)
{
	if (channel < 1 || channel > 8)
		return(IREADER_INVALID); 

    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();
    IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(MUX_MODULE));
    IF_ERROR_RETURN(((IReader *)handle)->IReaderRescanSlave(channel - 1));
    IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(READER_MODULE));
	((IReader *)handle)->IReaderGiveMutex();
    return IREADER_SUCCESS;

}

Int32 __cdecl IReaderApiReadTags(void * handle, int *tagcount, struct taginfo *tagrbuf)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderReadTags(tagcount, tagrbuf);
		
	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error); 
}

Int32 __cdecl IReaderApiReadTagsAtomic(void * handle, int antid, int *tagcount, struct taginfo *tagrbuf)
{
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();

    // Select the antenna to be read    
    IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(MUX_MODULE));
    IF_ERROR_RETURN(((Muxdev *)handle)->SetAntPort(antid - 1));
    IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(READER_MODULE));

    // Set the Read output power (power is already stored in driver variable) the to the M5e Reader module
    // This step can be skipped if the reading power for all the antennas is the same  
    IF_ERROR_RETURN(((IReader *)handle)->IReaderSetPowerLevel(antid - 1, 0, 1));
    
    // Now reguest the M5e to start reading the tags
    IF_ERROR_RETURN(((IReader *)handle)->IReaderReadTags(tagcount, tagrbuf));

	((IReader *)handle)->IReaderGiveMutex();
    return IREADER_SUCCESS;
}

Int32 __cdecl IReaderApiReadTagsMetaDataRSSI(void * handle, int antid, int pwr, int *tagcount, struct taginfo_rssi *tagrbuf)
{
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();

    // Select the antenna to be read    
    IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(MUX_MODULE));
    IF_ERROR_RETURN(((Muxdev *)handle)->SetAntPort(antid - 1));
    IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(READER_MODULE));

    // Set the Read output power (power is already stored in driver variable) the to the M5e Reader module
    // This step can be skipped if the reading power for all the antennas is the same  
    IF_ERROR_RETURN(((IReader *)handle)->IReaderSetPowerLevel(antid - 1, pwr, 1));
    
    // Now reguest the M5e to start reading the tags
    IF_ERROR_RETURN(((IReader *)handle)->IReaderReadTagsMetaDataRSSI(tagcount, tagrbuf));

	((IReader *)handle)->IReaderGiveMutex();
    return IREADER_SUCCESS;
}

Int32 __cdecl IReaderApiWriteTag(void * handle, int pwr, int timeout, unsigned char *tagid)
{
	int error;
	IReader *devHandle;
    int tagcount;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error != IREADER_SUCCESS)
	{
	    ((IReader *)handle)->IReaderGiveMutex();
	    return(error); 
		
	}
	devHandle = (IReader *)handle;
	error = devHandle->IReaderGetTagCount(&tagcount);
	if (error != IREADER_SUCCESS)
	{
	    ((IReader *)handle)->IReaderGiveMutex();
	    return(error); 
		
	}
    if ( 1 != tagcount )
    {
	    ((IReader *)handle)->IReaderGiveMutex();
        return (IREADER_WRITE_TAG_DENY);
    }
	error = devHandle->IReaderSetWritePowerLevel(pwr);
	if (error != IREADER_SUCCESS)
	{
	    ((IReader *)handle)->IReaderGiveMutex();
	    return(error); 
		
	}
	error = devHandle->IReaderWriteTag(timeout, tagid);
	((IReader *)handle)->IReaderGiveMutex();
    return (error);
}


Int32 __cdecl IReaderApiSelectAnt(void * handle, int antid)
{
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();
    IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(MUX_MODULE));
    IF_ERROR_RETURN(((Muxdev *)handle)->SetAntPort(antid - 1));
    IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(READER_MODULE));
	((IReader *)handle)->IReaderGiveMutex();
    return IREADER_SUCCESS;
}

Int32 __cdecl IReaderApiSelectLed(void * handle, int ledid)
{
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();
    IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(MUX_MODULE));
    IF_ERROR_RETURN(((Muxdev *)handle)->SetLedPort(ledid));
    IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(READER_MODULE));
	((IReader *)handle)->IReaderGiveMutex();
    return IREADER_SUCCESS;
}

Int32 __cdecl IReaderApiReadTagsAll(void * handle)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderReadTagsAll();
		
	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error); 
}

Int32 __cdecl IReaderApiSetPowerLevel(void * handle, int antid, int pwr, int doset)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderSetPowerLevel(antid-1, pwr, doset);
		
	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error); 
}

Int32 __cdecl IReaderApiGetPowerLevel(void * handle, int antid, int *pwr)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderGetPowerLevel(antid-1, pwr);
		
	}
	((IReader *)handle)->IReaderGiveMutex();
	// *pwr = 2500;
	return(error); 
}

Int32 __cdecl IReaderApiTagSearchTimeout(void * handle, int timeout)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderTagSearchTimeout(timeout);
	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error); 
}
