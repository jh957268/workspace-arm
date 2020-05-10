#include "stdio.h"
#include "time.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "iReaderapi.h"

#define HEADER_SIZE		256

int read_upd_header(char *upd_header, int len);


void send_resp(int code);

	const char *cgi_env1;
	const char *cgi_env2;
	const char *cgi_env3;
	const char *cgi_env;
	
char upd_header[HEADER_SIZE + 4];
int inserttag_main(	IReader *handle) 
{
    char c;
	//FILE *fp;
	
	cgi_env = getenv("QUERY_STRING");
	cgi_env1 = getenv("REQUEST_METHOD");
	cgi_env2 = getenv("CONTENT_TYPE");
	cgi_env3 = getenv("CONTENT_LENGTH");
	int input_len = atoi(cgi_env3);
	int iResult, ret;
	
	iResult = read_upd_header(upd_header, input_len);
	if (iResult != input_len)
	{
		send_resp(-1);
		return 0;
	}
	
   //fp = fopen("/home/joohong/upload/upload.txt", "w");
   //if (fp != NULL)
    //{
	//	fwrite(upd_header, 1, 66, fp);
	//	fclose(fp);	
    //}
	
	upd_header[input_len] = 0;
	
	// Call the API with upd_header as passing parameter
	ret = IReaderApiDBInsertTag(handle, upd_header);    // select all limit 0 offset 0 from table
	if (IREADER_SUCCESS != ret)
	{
		//printf("command fails\n");
		input_len = 999;
	}

	IReaderApiClose(handle);
	send_resp(input_len);
    fflush(stdout);
	
	return 0;
}

int read_upd_header(char *buffer, int len)
{
	int remain_len = len;
	int iResult = 0;
	
	while (remain_len)
	{
		iResult = fread(&buffer[remain_len-remain_len], 1, remain_len, stdin);
		if ((iResult == 0) || (iResult == -1))
		{
			return (iResult);
		}
		remain_len -= iResult;
	}
	return (len);
}

void send_resp(int code)
{
	printf("<html>");
//printf("Content-type:text/html\n");
printf("<head>");
printf("<title>Hello Word - First CGI Program</title>");
printf("</head>");
printf("<body>");
printf("%s !! %s !! %s !! %s", cgi_env, cgi_env1, cgi_env2, cgi_env3);

printf("<h2><font color=\"blue\">parameter value = %s</font></h2>", upd_header);
printf("<h2><font color=\"red\">Hello Word! This is my first CGI program code = %d</font></h2>", code);
printf("</body>");
printf("</html>");
fflush(stdout);
}
