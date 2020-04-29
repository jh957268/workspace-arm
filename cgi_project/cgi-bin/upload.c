#include "stdio.h"
#include "time.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define HEADER_SIZE		256

int read_upd_header(char *upd_header);


void send_resp(int code);

	const char *cgi_env1;
	const char *cgi_env2;
	const char *cgi_env3;
char upd_header[HEADER_SIZE + 4];
char upd_header_reverse[HEADER_SIZE + 4];
int main(void) 
{
    char c;
    FILE *fp;
	cgi_env1 = getenv("REQUEST_METHOD");
	cgi_env2 = getenv("CONTENT_TYPE");
	cgi_env3 = getenv("CONTENT_LENGTH");
	int input_len = atoi(cgi_env3);
	int iResult;
	char upd_header[HEADER_SIZE + 4];
	
	iResult = read_upd_header(upd_header);
	if (iResult != HEADER_SIZE)
	{
		send_resp(-1);
		return 0;
	}
	
	upd_header[256] = 0;
	
	
	
    fp = fopen("/home/joohong/upload/upload.txt", "w");
    if (fp == NULL)
    {
        //printf("cannot open file!!!");
		send_resp(-2);
        fflush(stdout);
		return 0;
    }
    //setvbuf(stdout, NULL, _IOLBF, 0);
	
	char *start = strstr(upd_header, "\r\n\r\n");
	if (start == 0)
	{
		send_resp(-2);
        fflush(stdout);
		return 0;
	}
	
	start += 4;
	
	fwrite(start, 1, &upd_header[256] - start, fp);
	
    while ((input_len - (2 * HEADER_SIZE)) > 0)
    {
		c = fgetc(stdin);
		fputc(c, fp);
//        fputc( c, stdout);
//        fflush(stdout);
        //if (feof(stdin))
        //{
        //    break;
        //}
		input_len--;
//	printf("</body>");
//	printf("</html>");
    }
	// read the rest 256 byte
	iResult = read_upd_header(upd_header);
	if (iResult != HEADER_SIZE)
	{
		send_resp(-1);
    	if (fp)
    	{
       		fclose(fp);
    	}
		return 0;
	}
	
	upd_header[256] = 0;
#if 1
	for (int i = 0; i < HEADER_SIZE	; i++)
	{
		upd_header_reverse[HEADER_SIZE - 1 - i] = upd_header[i];
	}
	upd_header_reverse[HEADER_SIZE] = 0;
	start = strstr(upd_header_reverse, "----\n\r");
	if (start == 0)
	{
		fwrite(&upd_header[0], 1, HEADER_SIZE, fp);
		send_resp(-4);
        fflush(stdout);
		return 0;
	}

	start += 6;
	int diff = (int)(start - &upd_header_reverse[0]);
#endif
	fwrite(&upd_header[0], 1, HEADER_SIZE - diff, fp);


    if (fp)
    {
       fclose(fp);
    }
	send_resp(0);
    return 0;
}

int read_upd_header(char *buffer)
{
	int len = HEADER_SIZE;
	int iResult = 0;
	
	while (len)
	{
		iResult = fread(&buffer[HEADER_SIZE-len], 1, len, stdin);
		if ((iResult == 0) || (iResult == -1))
		{
			return (iResult);
		}
		len -= iResult;
	}
	return (HEADER_SIZE);
}

void send_resp(int code)
{
	printf("<html>");
//printf("Content-type:text/html\n");
printf("<head>");
printf("<title>Hello Word - First CGI Program</title>");
printf("</head>");
printf("<body>");
printf("%s !! %s !! %s", cgi_env1, cgi_env2, cgi_env3);
printf("<h2><font color=\"red\">Hello Word! This is my first CGI program code = %d, %s</font></h2>", code,upd_header);
printf("</body>");
printf("</html>");
fflush(stdout);
}
