#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 

#include <sys/stat.h> 

#include <fcntl.h> 
#include "define.h"
#include "iReadererror.h"
#include "iReader.h"
#include "iReaderapi.h"

#define __cdecl

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
	if ( 0 == handle )
	{
		return (IREADER_HANDLE_INVALID);
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

void * __cdecl IReaderApiCreate(void)
{
	int i;
	IReader *devHandle;

	if (iReaderInit == 0)
	{
		for (i = 0; i < MAX_IREADER_DEVICES; i++)
			iReaderHandle[i] = (void *)NULL;
		iReaderInit = 1;
	}

	devHandle = new IReader();

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

//static char debug_buffer[128];
IReader * __cdecl IReaderApiInit(void)
{
	int i;
	IReader *devHandle;
	int error = IREADER_SUCCESS;

    devHandle = (IReader *)IReaderApiCreate();
    if (NULL == devHandle)
    {
	    return((UInt32)NULL); 
    }
    
	error = devHandle->IReaderInit();

    if (error != IREADER_SUCCESS)
	{
	    IReaderHandleEmptySlot(devHandle);
	    delete(devHandle);
	    return(NULL);
	}

    // for (i = 0; i < 256; i++)
	//    devHandle->IReaderSetPowerLevel(i, 2500, 0);  // setup dedfault power level 25dBm
	devHandle->IReaderTagSearchTimeout(80);     // default 80ms
    
	// devHandle->IReaderGetAntMap();      //  setup the antenna map at iReader object 
	devHandle->IReaderCreateMutex(); 
    
	return(devHandle);
}

Int32 __cdecl IReaderApiConnect(void * handle, char *remote)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderConnect(remote);
	}
	return(error); 
}

Int32 __cdecl IReaderApiGetAntMap(void * handle, char *map)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderGetAntMap(map);
		
	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error); 
}

Int32 __cdecl IReaderApiSetAntScanMap(void * handle, unsigned char *ant_map)
{
	int error = IREADER_SUCCESS;;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderSetAntScanMap(ant_map);
		
	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error); 
}

Int32 __cdecl IReaderApiGetAntScanMap(void * handle, unsigned char *ant_map)
{
	int error = IREADER_SUCCESS;;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderGetAntScanMap(ant_map);
		
	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error); 
}

Int32 __cdecl IReaderApiGetAntList(void * handle, int *antCount, int *antList)
{
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();
    //IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(MUX_MODULE));
    IF_ERROR_RETURN(((IReader *)handle)->IReaderGetAntList(antCount, antList));
    //IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(READER_MODULE));
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

Int32 __cdecl IReaderApiGetScanAntList(void * handle, int *antCount, int *antList)
{
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();
    //IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(MUX_MODULE));
    IF_ERROR_RETURN(((IReader *)handle)->IReaderGetScanAntList(antCount, antList));
    //IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(READER_MODULE));
	((IReader *)handle)->IReaderGiveMutex();
    return IREADER_SUCCESS;
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
		delete(devHandle);
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
    //IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(MUX_MODULE));
    IF_ERROR_RETURN(((IReader *)handle)->IReaderRescanSlave(chn));
    //IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(READER_MODULE));
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

Int32 __cdecl IReaderApiGetRegion(void * handle, int *region)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderGetRegion(region);

	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error);
}

Int32 __cdecl IReaderApiDBSelectAll(void * handle, int limit, int offset, int table)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderDBSelectAll(limit, offset, table);

	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error);
}

Int32 __cdecl IReaderApiDBInsertTag(void * handle, char * tag_str)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderDBInsertTag(tag_str);

	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error);
}


Int32 __cdecl IReaderApiGetSearchTimeout(void * handle, int *region)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderGetSearchTimeout(region);

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
    //IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(MUX_MODULE));
    IF_ERROR_RETURN(((IReader *)handle)->IReaderRescanSlave(channel - 1));
    //IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(READER_MODULE));
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
		error = devHandle->IReaderReadTags(0, tagcount, tagrbuf);
		
	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error); 
}

Int32 __cdecl IReaderApiReadTagsAtomic(void * handle, int antid, int *tagcount, struct taginfo *tagrbuf)
{
 
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderReadTags(antid -1, tagcount, tagrbuf);
		
	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error);  
}

Int32 __cdecl IReaderApiReadTagsMetaDataRSSI(void * handle, int antid, int pwr, int *tagcount, struct taginfo_rssi *tagrbuf)
{
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();

    // Now reguest the M5e to start reading the tags
    IF_ERROR_RETURN(((IReader *)handle)->IReaderReadTagsMetaDataRSSI(antid, pwr, tagcount, tagrbuf));

	((IReader *)handle)->IReaderGiveMutex();
    return IREADER_SUCCESS;
}

Int32 __cdecl IReaderApiGetTagsMetaDataRSSI(void * handle, int *antid, int *tagcount, struct taginfo_rssi *tagrbuf)
{
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	// ((IReader *)handle)->IReaderTakeMutex();			No need, different socket

    // Now reguest the M5e to start reading the tags
    IF_ERROR_RETURN(((IReader *)handle)->IReaderGetTagsMetaDataRSSI(antid, tagcount, tagrbuf));

	// ((IReader *)handle)->IReaderGiveMutex();
    return IREADER_SUCCESS;
}

Int32 __cdecl IReaderApiGetTagDBRecord(void * handle, char *dbrecord, int *tagcount)
{
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	// ((IReader *)handle)->IReaderTakeMutex();			No need, different socket

    // Now reguest the M5e to start reading the tags
    IF_ERROR_RETURN(((IReader *)handle)->IReaderGetTagDBRecord(dbrecord, tagcount));

	// ((IReader *)handle)->IReaderGiveMutex();
    return IREADER_SUCCESS;
}

Int32 __cdecl IReaderApiGetTags(void * handle, int *antid, int *tagcount, struct taginfo *tagrbuf)
{
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	//((IReader *)handle)->IReaderTakeMutex();

    // Now reguest the M5e to start reading the tags
    IF_ERROR_RETURN(((IReader *)handle)->IReaderGetTags(antid, tagcount, tagrbuf));

	//((IReader *)handle)->IReaderGiveMutex();
    return IREADER_SUCCESS;
}

Int32 __cdecl IReaderApiStartExecutor(void * handle, int flag, int fd)
{
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();

    // Now reguest the M5e to start reading the tags
    IF_ERROR_RETURN(((IReader *)handle)->IReaderStartExecutor(flag, fd));

	((IReader *)handle)->IReaderGiveMutex();
    return IREADER_SUCCESS;
}

Int32 __cdecl IReaderApiWriteTagData(void * handle, int antid, int pwr, int membank, Uint32 addr, unsigned char *data, int datalen, unsigned char *tagid, unsigned char *passwd)
{
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();

    // Select the antenna to be written    
    //IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(MUX_MODULE));
    //IF_ERROR_RETURN(((Muxdev *)handle)->SetAntPort(antid - 1));
    //IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(READER_MODULE));

    // Set the Read output power (power is already stored in driver variable) the to the M5e Reader module
    // This step can be skipped if the reading power for all the antennas is the same  
    IF_ERROR_RETURN(((IReader *)handle)->IReaderSetWritePowerLevel(pwr));

    IF_ERROR_RETURN(((IReader *)handle)->IReaderWriteTagData(membank, addr, data, datalen, tagid, passwd));

	((IReader *)handle)->IReaderGiveMutex();
    return IREADER_SUCCESS;

}

Int32 __cdecl IReaderApiReadTagData(void * handle, int antid, int membank, Uint32 addr, unsigned char *data, int datalen, unsigned char *tagid, unsigned char *passwd)
{
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();

    // Select the antenna to be written    
    //IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(MUX_MODULE));
    //IF_ERROR_RETURN(((Muxdev *)handle)->SetAntPort(antid - 1));
    //IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(READER_MODULE));

    // Set the Read output power (power is already stored in driver variable) the to the M5e Reader module
    // This step can be skipped if the reading power for all the antennas is the same  
    // IF_ERROR_RETURN(((IReader *)handle)->IReaderSetPowerLevel(antid - 1, 0, 1));

    IF_ERROR_RETURN(((IReader *)handle)->IReaderReadTagData(membank, addr, data, datalen, tagid, passwd));

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
	Int32 ret;
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();
	ret = ((IReader *)handle)->setantport(antid - 1);
    // IF_ERROR_RETURN(((Muxdev *)handle)->SetAntPort(antid - 1));
    // ((Muxdev *)handle)->SetAntPort(antid - 1);
	((IReader *)handle)->IReaderGiveMutex();
    return ret;
}

Int32 __cdecl IReaderApiSetScanAntId(void * handle, int antid, int flag)
{
	Int32 ret;
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();
	ret = ((IReader *)handle)->setscanantid(antid - 1, flag);
    // IF_ERROR_RETURN(((Muxdev *)handle)->SetAntPort(antid - 1));
    // ((Muxdev *)handle)->SetAntPort(antid - 1);
	((IReader *)handle)->IReaderGiveMutex();
    return ret;
}

Int32 __cdecl IReaderApiSelectLed(void * handle, int ledid)
{
    IF_ERROR_RETURN(IReaderHandleValid(handle));
	((IReader *)handle)->IReaderTakeMutex();
    //IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(MUX_MODULE));
    //IF_ERROR_RETURN(((Muxdev *)handle)->SetLedPort(ledid));
    //IF_ERROR_RETURN(((MuxClient *)handle)->SelectModule(READER_MODULE));
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

Int32 __cdecl IReaderApiSetPowerLevel(void * handle, int antid, int pwr)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);
	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->IReaderSetPowerLevel(antid, pwr);
		
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

Int32 __cdecl IReaderApiTempProtectSet(void * handle, int protect)
{
	int error;
	IReader *devHandle;

	error = IReaderHandleValid(handle);

	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->SetEquipTempProtect(protect);
	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error); 

}

Int32 __cdecl IReaderApiEquipTempGet(void * handle, int *temp)
{
	int error;
	IReader *devHandle;
	*temp = 0;

	error = IReaderHandleValid(handle);

	((IReader *)handle)->IReaderTakeMutex();
	if (error == IREADER_SUCCESS)
	{
		devHandle = (IReader *)handle;
		error = devHandle->GetEquipTemp(temp);
	}
	((IReader *)handle)->IReaderGiveMutex();
	return(error); 
}
