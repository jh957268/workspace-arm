#include "stdio.h"
#include "time.h"
#include <unistd.h>

char epcdata[64];

int main(void) 
{
    time_t now;
    int toggle = 0;
    //setvbuf(stdout, NULL, _IOLBF, 0);
    sprintf(epcdata, "0123456789abcdef01234567~");
    for (;;)
    {

    	printf("Content-Type: text/event-stream\r\n");
    	printf("Cache-Control: no-cache\n\n");
    	//printf("Content-Type: text/event-stream\r\n");
	printf("Event: the server time \r\n");
        now = time(NULL);
        if (toggle == 0)
	{
	    printf("data:0123456789abcdef01234567~%ld\r\n\r\n",now);
	}
        if (toggle == 1)
	{
	    printf("data:0123456789abcdef01234568~%ld\r\n\r\n",now);
	}
        if (toggle == 2)
	{
	    printf("data:0123456789abcdef01234569~%ld\r\n\r\n",now);
	}
        if (toggle == 3)
	{
	    printf("data:0123456789abcdef0123456a~%ld\r\n\r\n",now);
	}
	toggle += 1;
	toggle &= 3;
//	printf("</body>");
//	printf("</html>");
	fflush(stdout);
	sleep(1);
    }
}
