#ifndef __IREADERAPI_H__
#define __IREADERAPI_H__

#include "iReader.h"
#include "iReadererror.h"


extern "C"  void * IReaderApiInit(char *remote, int region);
extern "C"  Int32  IReaderApiGetAntMap(void * handle);
extern "C"  Int32  IReaderApiClose(void * handle);
extern "C"  Int32  IReaderApiGetAntList(void * handle, int *antCount, int *antList);
extern "C"  Int32  IReaderApiSetRegion(void * handle, int region);
extern "C"  Int32  IReaderApiSyncChannel(void * handle, int region);
extern "C"  Int32  IReaderApiReadTags(void * handle, int *tagcount, struct taginfo *tagrbuf);
extern "C"  Int32  IReaderApiReadTagsAtomic(void * handle, int antid, int *tagcount, struct taginfo *tagrbuf);
extern "C"  Int32  IReaderApiReadTagsMetaDataRSSI(void * handle, int antid, int pwr, int *tagcount, struct taginfo_rssi *tagrbuf);
extern "C"  Int32  IReaderApiWriteTag(void * handle, int pwr, int timeout, unsigned char *tagid);
extern "C"  Int32  IReaderApiSetPowerLevel(void * handle, int antid, int pwr, int doset);
extern "C"  Int32  IReaderApiGetPowerLevel(void * handle, int antid, int *pwr);
extern "C"  Int32  IReaderApiSelectAnt(void * handle, int antid);
extern "C"  Int32  IReaderApiTagSearchTimeout(void * handle, int timeout);
extern "C"  Int32  IReaderApiScanSlave(void * handle, int channel);
extern "C"  Int32  IReaderApiSelectLed(void * handle, int ledid);

#endif
