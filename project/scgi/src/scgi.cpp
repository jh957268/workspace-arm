//============================================================================
// Name        : scgi.cpp
// Author      : joo
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

/*
 * simple and trivial SCGI server with hard-coded results for use in unit tests
 * - listens on STDIN_FILENO (socket on STDIN_FILENO must be set up by caller)
 * - processes a single SCGI request at a time
 * - arbitrary limitation: reads request headers netstring up to 64k in size
 * - expect recv data for request headers netstring every 10ms or less
 * - no read timeouts for request body; might block reading request body
 * - no write timeouts; might block writing response
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0
#endif

static int finished;
static char buf[65536];


static char *
scgi_getenv(char *r, const unsigned long rlen, const char * const name)
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


static void
scgi_process (const int fd)
{
    ssize_t rd = 0, offset = 0;
    int num_requests = 1;
    char *p = NULL, *r;
    unsigned long rlen;
    long long cl;

    assert(fd == STDOUT_FILENO); /*(required for response sent with printf())*/
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
				printf("Status: 200 OK\r\n\r\n");
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
			rd = recv(fd, buf, 100, 0);
			if ( rd <= 0)
			{
				printf("test123");
			}
		}
    }

    fflush(stdout);
    if (0 == num_requests) finished = 1;
}

class base {
  public:
    base()
    { cout<<"Constructing base \n"; }
    ~base()
    { cout<<"Destructing base \n"; }
};

class base1 {
  public:
    base1()
    { cout<<"Constructing base 1\n"; }
   virtual ~base1()
    { cout<<"Destructing base 1 \n"; }
};

class derived: public base {
  public:
    derived()
    { cout<<"Constructing derived \n"; }
    ~derived()
    { cout<<"Destructing derived \n"; }
};

class derived1: public base1 {
  public:
    derived1()
    { cout<<"Constructing derived 1 \n"; }
    ~derived1()
    { cout<<"Destructing derived 1\n"; }
};

#define millis()	1000

class elapsedMillis
{
private:
	unsigned long ms;
public:
	elapsedMillis(void) { ms = millis(); }
	elapsedMillis(unsigned long val) { ms = millis() - val; }
	elapsedMillis(const elapsedMillis &orig) { ms = orig.ms; }
	operator unsigned long () const { return millis() - ms; }
	elapsedMillis & operator = (const elapsedMillis &rhs) { ms = rhs.ms; return *this; }
	elapsedMillis & operator = (unsigned long val) { ms = millis() - val; return *this; }
	elapsedMillis & operator -= (unsigned long val)      { ms += val ; return *this; }
	elapsedMillis & operator += (unsigned long val)      { ms -= val ; return *this; }
	elapsedMillis operator - (int val) const           { elapsedMillis r(*this); r.ms += val; return r; }
	elapsedMillis operator - (unsigned int val) const  { elapsedMillis r(*this); r.ms += val; return r; }
	elapsedMillis operator - (long val) const          { elapsedMillis r(*this); r.ms += val; return r; }
	elapsedMillis operator - (unsigned long val) const { elapsedMillis r(*this); r.ms += val; return r; }
	elapsedMillis operator + (int val) const           { elapsedMillis r(*this); r.ms -= val; return r; }
	elapsedMillis operator + (unsigned int val) const  { elapsedMillis r(*this); r.ms -= val; return r; }
	elapsedMillis operator + (long val) const          { elapsedMillis r(*this); r.ms -= val; return r; }
	elapsedMillis operator + (unsigned long val) const { elapsedMillis r(*this); r.ms -= val; return r; }
};


#include <iostream>
#include <vector>

std::vector<std::string> unique_names(const std::vector<std::string>& names1, const std::vector<std::string>& names2)
{
    // throw std::logic_error("Waiting to be implemented");
	int found = 0;

    //std::vector<std::string> names3 = names1;
    std::vector<std::string> names3;
    for (std::vector<std::string>::const_iterator it = names1.begin() ; it != names1.end(); ++it)
    {
    	found = 0;
        for (std::vector<std::string>::iterator it3 = names3.begin() ; it3 != names3.end(); ++it3)
        {
        	if (*it == *it3)
        	{
        		found = 1;;
        	}
        }
        if ( 0 == found)
        {
        	names3.push_back(*it);
        }

    }
    for (std::vector<std::string>::const_iterator it = names2.begin() ; it != names2.end(); ++it)
    {
    	std::string name = *it;
    	found = 0;
        for (std::vector<std::string>::iterator it1 = names3.begin() ; it1 != names3.end(); ++it1)
        {
        	if (*it1 == name)
        	{
        		found = 1;
        		break;
        	}
        }
        if ( 0 == found)
        {
    		names3.push_back(name);
        }
    }
    return names3;
}

struct MyException : public exception {
   const char * what () const throw () {
      return "C++ Exception";
   }
};


class TextInput
{
public:

	virtual ~TextInput(){}
    virtual void add(char c) { }

    virtual std::string getValue() { return NULL; }
};

class NumericInput : public TextInput
{
public:
     NumericInput() {val = "0";}

    std::string val;

    void add(char c)
    {
        if (isdigit(c))
        {
           char arr[4];

           sprintf(arr, "%c", c);
           val += std::string(arr);
        }
    }
    std::string getValue() { return (val); }
};


#define SA struct sockaddr
int
main (void)
{
    int fd, conn_fd;
    //fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK);
    //close(STDOUT_FILENO); /*(so that accept() returns fd to STDOUT_FILENO)*/

    std::string str("1234");

    std::cout << str << '\n';

    TextInput* input = new NumericInput();
    input->add('1');
    input->add('a');
    input->add('0');
    std::cout << input->getValue();

    try {
       throw MyException();
    } catch(MyException& e) {
       std::cout << "MyException caught" << std::endl;
       std::cout << e.what() << std::endl;
    } catch(std::exception& e) {
       //Other errors
    }

    try
    {
      throw 20;
    }
    catch (int e)
    {
      cout << "An exception occurred. Exception Nr. " << e << '\n';
    }

    std::vector<std::string> names1 = {"Ava", "Emma", "Olivia"};
    std::vector<std::string> names2 = {"Olivia", "Sophia", "Emma"};

    std::vector<std::string> result = unique_names(names1, names2);
    for(auto element : result)
    {
        std::cout << element << ' '; // should print Ava Emma Olivia Sophia
    }


    elapsedMillis blink;
    elapsedMillis led;

    blink = 100;
    led = 500;
    if (blink > 250)
    {
    	printf("Blink elapsed\n");
    }

    if (blink < 50)
    {
    	printf("Blink smaller\n");
    }

    blink = led;

     derived *d = new derived();
	 base *b = d;
	 delete b;

	 d = new derived();
	 delete d;

     derived1 *d1 = new derived1();
	 base1 *b1 = d1;
	 delete b1;

	 d1 = new derived1();
	 delete d1;

#if 0
    static FILE *fp;

    fp = fopen("/home/joohong/upload/llrp.log", "a");
    if (fp == NULL)
    {
    	exit(-1);
    }
    //fprintf(fp, "receive signal SIG NUM = %d\n", signum);
    //fclose(fp);
    fd = fileno(fp);

    int stdout_copy = dup(STDOUT_FILENO);

    close(1); /* close the stdout associated with screen */

    int fd1 = dup(fd); /* one.txt is new stdout for the process */

    printf("This is dup test-1\n");
    fflush(stdout);
     close(fd);

    dup2(stdout_copy, 1);
    close (stdout_copy);
    printf("This is dup test-2\n");
    fflush(stdout);
    exit(0);

#endif

    int sockfd, connfd, len;
     struct sockaddr_in servaddr, cli;

     // socket create and verification
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd == -1) {
         printf("socket creation failed...\n");
         exit(0);
     }
     //else
     //    printf("Socket successfully created..\n");
     bzero(&servaddr, sizeof(servaddr));

     // assign IP, PORT
     servaddr.sin_family = AF_INET;
     servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
     servaddr.sin_port = htons(9998);

     // Binding newly created socket to given IP and verification
     if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
         printf("socket bind failed...\n");
         exit(0);
     }
     //else
     //    printf("Socket successfully binded..\n");

     // Now server is ready to listen and verification
     if ((listen(sockfd, 5)) != 0) {
         printf("Listen failed...\n");
         exit(0);
     }
     //else
     //    printf("Server listening..\n");
     len = sizeof(cli);

    do {
    	conn_fd = accept(sockfd, (SA*)&cli, &len);
        // fd = accept(sockfd, NULL, NULL);
        if (conn_fd < 0)
        {
        	perror("accept STDIN");
            continue;
        }
        // fd = STDOUT_FILENO;
        close(STDOUT_FILENO);
        fd = dup(conn_fd);
        assert(fd == STDOUT_FILENO);
        scgi_process(fd);
    } while (fd > 0 ? 0 == close(fd) && !finished : errno == EINTR);

    return 0;
}

