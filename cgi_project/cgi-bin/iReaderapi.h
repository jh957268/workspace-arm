#ifndef __IREADERAPI_H__
#define __IREADERAPI_H__

#include "iReader.h"
#include "iReadererror.h"

#define __declspec( dllexport )
#define __cdecl

extern "C" __declspec( dllexport )IReader * __cdecl IReaderApiInit(void);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiConnect(void * handle, char *remote);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiGetAntMap(void *handle, char *map);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiSetAntScanMap(void *handle, unsigned char *ant_map);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiGetAntScanMap(void *handle, unsigned char *ant_map);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiClose(void *handle);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiGetAntList(void *handle, int *antCount, int *antList);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiGetScanAntList(void *handle, int *antCount, int *antList);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiSetRegion(void *handle, int region);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiGetRegion(void *handle, int *region);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiDBSelectAll(void *handle);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiGetSearchTimeout(void *handle, int *timeout);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiSyncChannel(void *handle, int region);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiReadTags(void *handle, int *tagcount, struct taginfo *tagrbuf);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiReadTagsAtomic(void *handle, int antid, int *tagcount, struct taginfo *tagrbuf);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiReadTagsMetaDataRSSI(void *handle, int antid, int pwr, int *tagcount, struct taginfo_rssi *tagrbuf);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiGetTagsMetaDataRSSI(void *handle, int *antid, int *tagcount, struct taginfo_rssi *tagrbuf);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiGetTags(void *handle, int *antid, int *tagcount, struct taginfo *tagrbuf);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiStartExecutor(void *handle, int flag);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiWriteTag(void *handle, int pwr, int timeout, unsigned char *tagid);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiSetPowerLevel(void *handle, int antid, int pwr, int doset);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiGetPowerLevel(void *handle, int antid, int *pwr);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiSelectAnt(void *handle, int antid);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiTagSearchTimeout(void *handle, int timeout);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiScanSlave(void *handle, int channel);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiSelectLed(void *handle, int ledid);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiWriteTagData(void *handle, int antid, int pwr, int membank, Uint32 addr, unsigned char *data, int datalen, unsigned char *tagid, unsigned char *passwd);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiReadTagData(void *handle, int antid, int membank, Uint32 addr, unsigned char *data, int datalen, unsigned char *tagid, unsigned char *passwd);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiSetScanAntId(void *handle, int antid, int flag);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiTempProtectSet(void *handle, int protect);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiEquipTempGet(void *handle, int *temp);
extern "C" __declspec( dllexport )Int32 __cdecl IReaderApiGetTagDBRecord(void * handle, char *dbrecord, int *tagcount);

#endif
