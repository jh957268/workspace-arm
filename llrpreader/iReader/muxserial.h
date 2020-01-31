/* ---------------------------------------------------------------------- */
#ifndef MUXSERIAL_H
#define MUXSERIAL_H

#include <stdio.h>
//#include <windows.h>
#include "muxdev.h"
#include "OwMutex.h"

#define HANDLE	int

/* -------------------------------------------------------------------- */
/* -----------------------------  Muxserial  ---------------------------- */
/* -------------------------------------------------------------------- */
class Muxserial
{
    // -------------------------------------------------------- //
protected:
    char              port[12];                      // port name "com1",...
    int               rate;                          // baudrate
    serial_parity     parityMode;
    HANDLE            serial_handle;                 // ...

	OwMutex			  *mpMutex;

    // ++++++++++++++++++++++++++++++++++++++++++++++
    // .................. EXTERNAL VIEW .............
    // ++++++++++++++++++++++++++++++++++++++++++++++
public:
                  Muxserial(char *comPort);
                 ~Muxserial();
    int           Connect          (void);
    int           sendChar         (char c);
    int     	  sendArray        (uint8_t *buffer, int len);
    int           getChar          (unsigned char *ch);
    int           getArray         (char *buffer, int len);
    int           getNbrOfBytes    (void);
    void          Disconnect       (void);
	void          clearRcv		   (void);
};
/* -------------------------------------------------------------------- */
extern Muxserial *Ser1;
extern Muxserial *Ser2;

#endif // TSERIAL_H


