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
	
	IAgent_Executor::getInstance()->SetCallbackHandler(this);
	int fd;

	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK);

	//close(STDOUT_FILENO); /*(so that accept() returns fd to STDOUT_FILENO)*/

	while (true)
	{
        fd = clientSocket;
        //dup2(clientSocket, fd);
        //assert(fd == STDOUT_FILENO);
        scgi_process(fd);
        break;
	}
	DBG_PRINT( DEBUG_INFO, "SAgent[%d]:: Closed"NL, id );
	::close(clientSocket);
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

void
SAgent::scgi_process (const int fd)
{
    ssize_t rd = 0, offset = 0;
    int num_requests = 1;
    char *p = NULL, *r;
    unsigned long rlen;
    long long cl;

    // assert(fd == STDOUT_FILENO); /*(required for response sent with printf())*/
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);


    do {
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
    } while (NULL == (p = memchr(buf,':',offset)) && offset < 21);
    if (NULL == p)
        return; /* timeout or error receiving start of netstring */
    rlen = strtoul(buf, &p, 10);
    if (*buf == '-' || *p != ':' || p == buf || rlen == ULONG_MAX)
        return; /* invalid netstring (and rlen == ULONG_MAX is too long)*/
    if (rlen > sizeof(buf) - (p - buf) - 2)
        return; /* netstring longer than arbitrary limit we accept here */
    rlen += (p - buf) + 2;

    while ((ssize_t)rlen < offset) {
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
        else if (0 == rd)
            break;
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
            continue;
        else
            break;
    }
    if (offset < (ssize_t)rlen)
        return; /* timeout or error receiving netstring */
    if (buf[rlen-1] != ',')
        return; /* invalid netstring */
    rlen -= (p - buf) + 2;
    r = p+1;

    /* not checking for empty headers in SCGI request (empty values allowed) */

    /* SCGI request must contain "SCGI" header with value "1" */
    p = scgi_getenv(r, rlen, "SCGI");
    if (NULL == p || p[0] != '1' || p[1] != '\0')
        return; /* missing or invalid SCGI header */

    /* CONTENT_LENGTH must be first header in SCGI request; always required */
    if (0 != strcmp(r, "CONTENT_LENGTH"))
        return; /* missing CONTENT_LENGTH */

    errno = 0;
    cl = strtoll(r+sizeof("CONTENT_LENGTH"), &p, 10);
    if (*p != '\0' || p == r+sizeof("CONTENT_LENGTH") || cl < 0 || 0 != errno)
        return; /* invalid CONTENT_LENGTH */

    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);

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
				::send(fd, "Status: 200 OK\r\n\r\n", strlen("Status: 200 OK\r\n\r\n"), 0);
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
			::send(fd, "120", strlen("120"), 0);
			//for (int j = 0; j < 10; j++)
			//{
			//	sleep(3);
			//	::send(fd, "120", strlen("150"), 0);
			//}

		}
    }

    //fflush(stdout);
    if (0 == num_requests) finished = 1;
}

void
SAgent::TagEventCallback (const char *tag_data) 
{}


