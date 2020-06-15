/*
 * CFile1.c
 *
 * Created: 12/24/2013 9:16:01 PM
 *  Author: jhong
 */ 
#include "sAgent.h"

#include  "iReaderapi.h"
#include  "debug_print.h"
#include  "iAgent_executor.h"
#include "CSqlite.h"
#include "CAntenna.h"
#include "pwm.h"

#include <iostream>

using namespace std;

unsigned int SAgent::nextId = 0;

#define _time64		time

extern CSqlite *Sqlite_db;

//=============================================================================
// Constructor

SAgent::SAgent():
	OwTask( MEDIUM, 2048, "SAgent" ),
	isSocketClosed ( false ),
	id( ++nextId )
{
	DBG_PRINT( DEBUG_INFO, "SAgent[%d]:: Created"NL, id );
	idle = false;					// Assume it will be run right away to avoid race condition 
									// that the MntServer will use it again if another connection coming in
} // SAgent::SAgent()

void
SAgent::main
(
	OwTask *
)
{
	//time_t now;
	//char data[128];
	
	//IAgent_Executor::getInstance()->SetCallbackHandler(this);
	//int fd;
	//int stdout_save;
	int ret;

	//fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK);

	//close(STDOUT_FILENO); /*(so that accept() returns fd to STDOUT_FILENO)*/
	//stdout_save = dup(STDOUT_FILENO);
	//close(STDOUT_FILENO);
    //fd = dup(clientSocket);
    //dup2(clientSocket, fd);
    //assert(fd == STDOUT_FILENO);
	while (true)
	{
		cgi_string.clear();
        ret = scgi_process(clientSocket);
		
		if (-1 == ret)
		{
			IAgent_Executor::getInstance()->RemoveCallbackHandler(&cliObj);
			printf("SAgent Done test123 -- %d!\n", ret);
			perror("recv");			
			break;
		}

        if (0 != cgi_string.length())
        {
			cliObj.set_sock_descriptor(clientSocket);
			cliObj.done = 0;
			cliObj.process_cli_command(cgi_string);
        }
		if ( 1 == cliObj.done)
		{
			break;
		}
#if 0		
		// fcntl(clientSocket, F_SETFL, fcntl(clientSocket, F_GETFL) & ~O_NONBLOCK);
		ret = recv(clientSocket, buf, 100, 0);
		if ( ret <= 0)
		{
			printf("SAgent Done test123 -- %d!\n", ret);
			perror("recv");
			IAgent_Executor::getInstance()->RemoveCallbackHandler(&cliObj);
			break;
		}
#endif		
	}
    ::close(clientSocket);
    //dup2(stdout_save, 1);
    //close (stdout_save);

	DBG_PRINT( DEBUG_INFO, "SAgent[%d]:: Closed"NL, id );
	// ::close(clientSocket);
	idle = true;  // Once this is set to true, other connection (from LLRP_Mntserver) may use this object before the
	              // phtread exit if preemption "can" happen
}


void
SAgent::disconnect
(
)
{
	// Close TCP socket
	// Linux environment

	isSocketClosed = true;
	::close(clientSocket);
	DBG_PRINT(DEBUG_INFO, "SAgent[%d]:: close socket"NL, id );

	// Tell the controller not to send more indication to here anymore
	//AkMsgProcessor::getInstance()->unsetController( this );

	// Tell AkRec that there is one less controller now
	//AkRec::getInstance()->decControllerCount();

	// inform the AkMntServer that it is going idle
	//AkMntServer::getInstance()->complete( this );

} // SAgent::disconnect()

//=============================================================================
// setConnection
// This function is call by "AkMntServer" to pass down the file
// descriptor of a newly established TCP socket. It also gives a
// semaphore to release its pending thread.

void
SAgent::setConnection
(
	SOCKET  connectClient
)
{
	DBG_PRINT(DEBUG_INFO, "SAgent[%d]:: Socket %d assigned"NL, id, connectClient );

	clientSocket = connectClient;
	// sem.give();				 // Release the thread

} // SAgent::setConnection()

char *
SAgent::scgi_getenv(char *r, const unsigned long rlen, const char * const name)
{
    /* simple search;
     * if many lookups are done, then should use more efficient data structure*/
    char * const end = r+rlen;
    char *z;
    const size_t len = strlen(name);
    do {
        if (0 == strcmp(r, name)) return r+len+1;

        z = memchr(r, '\0', (size_t)(end-r));
        if (NULL == z) return NULL;
        z = memchr(z+1, '\0', (size_t)(end-r));
        if (NULL == z) return NULL;
        r = z+1;
    } while (r < end);
    return NULL;
}

int
SAgent::scgi_process (const int fd)
{
    ssize_t rd = 0, offset = 0;
    int num_requests = 1;
    char *p = NULL, *r;
    unsigned long rlen;
    long long cl;
	int ret = 0;

    // assert(fd == STDOUT_FILENO); /*(required for response sent with printf())*/
    // fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);


    do {
#if 0		
        struct pollfd pfd = { fd, POLLIN, 0 };
        switch (poll(&pfd, 1, 10)) { /* 10ms timeout */
          default: /* 1; the only pfd has revents */
            break;
          case -1: /* error */
          case  0: /* timeout */
            pfd.revents |= POLLERR;
            break;
        }
        if (!(pfd.revents & POLLIN))
            break;
        do {
            rd = recv(fd, buf+offset, sizeof(buf)-offset, MSG_DONTWAIT);
        } while (rd < 0 && errno == EINTR);
        if (rd > 0)
            offset += rd;
        else if (0 == rd) {
            p = memchr(buf, ':', offset);
            break;
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
            continue;
        else
            break;
#endif
        do {
            rd = recv(fd, buf+offset, sizeof(buf)-offset, 0);
        } while (rd < 0 && errno == EINTR);
		if (rd <= 0)
		{
			return (-1);
		}
		offset += rd;
    } while (NULL == (p = memchr(buf,':',offset)) && offset < 21);
    if (NULL == p)
        return (-1); /* time out or error receiving start of netstring */
    rlen = strtoul(buf, &p, 10);
    if (*buf == '-' || *p != ':' || p == buf || rlen == ULONG_MAX)
        return (-1); /* invalid netstring (and rlen == ULONG_MAX is too long)*/
    if (rlen > sizeof(buf) - (p - buf) - 2)
        return (-1); /* netstring longer than arbitrary limit we accept here */
    rlen += (p - buf) + 2;

    while ((ssize_t)rlen > offset) {
#if 0		
        struct pollfd pfd = { fd, POLLIN, 0 };
        switch (poll(&pfd, 1, 10)) { /* 10ms timeout */
          default: /* 1; the only pfd has revents */
            break;
          case -1: /* error */
          case  0: /* timeout */
            pfd.revents |= POLLERR;
            break;
        }
        if (!(pfd.revents & POLLIN))
            break;
#endif		
        do {
            rd = recv(fd, buf+offset, sizeof(buf)-offset, 0);
        } while (rd < 0 && errno == EINTR);
		
		if (rd <= 0)
		{
			return (-1);
		}
        // if (rd > 0)
        offset += rd;
#if 0		
        else if (0 == rd)
            break;
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
            continue;
        else
            break;
#endif
		
    }
    if (offset < (ssize_t)rlen)
        return (-1); /* timeout or error receiving netstring */
    if (buf[rlen-1] != ',')
        return(-1); /* invalid netstring */
    rlen -= (p - buf) + 2;
    r = p+1;

    /* not checking for empty headers in SCGI request (empty values allowed) */

    /* SCGI request must contain "SCGI" header with value "1" */
    p = scgi_getenv(r, rlen, "SCGI");
    if (NULL == p || p[0] != '1' || p[1] != '\0')
        return (-1); /* missing or invalid SCGI header */

    /* CONTENT_LENGTH must be first header in SCGI request; always required */
    if (0 != strcmp(r, "CONTENT_LENGTH"))
        return (-1); /* missing CONTENT_LENGTH */

    errno = 0;
    cl = strtoll(r+sizeof("CONTENT_LENGTH"), &p, 10);
    if (*p != '\0' || p == r+sizeof("CONTENT_LENGTH") || cl < 0 || 0 != errno)
        return (-1); /* invalid CONTENT_LENGTH */

    // fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);

    /* read,discard request body (currently ignored in these SCGI unit tests)
     * (make basic effort to read body; ignore any timeouts or errors here) */
    cl -= (offset - (r+rlen+1 - buf));
    while (cl > 0) {
        char x[8192];
        do {
            rd = recv(fd, x, (cl>(long long)sizeof(x)?sizeof(x):(size_t)cl), 0);
        } while (rd < 0 && errno == EINTR);
        if (rd <= 0)
            break;
        cl -= rd;
    }

    /*(from fcgi-responder.c, substituting scgi_getenv() for getenv())*/
    {
		if (NULL != (p = scgi_getenv(r, rlen, "QUERY_STRING"))) {
			if (0 == strcmp(p, "lf")) {
				printf("Status: 200 OK\n\n");
			} else if (0 == strcmp(p, "crlf")) {
				printf("Status: 200 OK\r\n\r\n");
			} else if (0 == strcmp(p, "slow-lf")) {
				printf("Status: 200 OK\n");
				fflush(stdout);
				printf("\n");
			} else if (0 == strcmp(p,"slow-crlf")) {
				printf("Status: 200 OK\r\n");
				fflush(stdout);
				printf("\r\n");
			} else if (0 == strcmp(p, "die-at-end")) {
				printf("Status: 200 OK\r\n\r\n");
				num_requests--;
			} else {
				cgi_string = p;
				//printf("Status: 200 OK\r\n\r\n");		// Let CCLI to print this message
			}
		} else {
			printf("Status: 500 Internal Foo\r\n\r\n");
		}

		if (0 == strcmp(p, "path_info")) {
			printf("%s", scgi_getenv(r, rlen, "PATH_INFO"));
		} else if (0 == strcmp(p, "script_name")) {
			printf("%s", scgi_getenv(r, rlen, "SCRIPT_NAME"));
		} else if (0 == strcmp(p, "var")) {
			p = scgi_getenv(r, rlen, "X_LIGHTTPD_FCGI_AUTH");
			printf("%s", p ? p : "(no value)");
		} else {
			//rd = recv(fd, buf, 100, 0);
			//if ( rd <= 0)
			//{
			//	printf("test123");
			//}
			//printf("180");
			//std::string cgi_string(r);
			//cliObj.process_cli_command(cgi_string);
			//for (int j = 0; j < 10; j++)
			//{
			//	OwTask::sleep(1000);
			//	::send(fd, "130", strlen("150"), 0);
			//}

		}
    }

    //fflush(stdout);
    if (0 == num_requests) finished = 1;
	return(ret);
}

void
SAgent::TagEventCallback (uint8_t *tag_data, int tag_cnt, int ant_id) 
{}



