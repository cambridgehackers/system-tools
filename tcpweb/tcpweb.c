// Copyright John Ankcorn, 2013
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/poll.h>

#define MAX_POLL	100
#define POLL_TIMEOUT	20 /* milliseconds */
static char temp_buf[4096];
static struct pollfd pollstr[MAX_POLL];
static int otherfd[MAX_POLL];
static struct sockaddr_in dest_addr[MAX_POLL];

static void memdump(unsigned char *p, int len, char *title)
{
int i = 0;

    while (len > 0) {
        if ((i & 0xf) == 0) {
                if (i > 0)
                printf("\n");
                printf("%s: ",title);
        }
        printf("%02x ", *p++);
        i++;
        len--;
    }
    printf("\n");
}
int get_fd(int sockfd)
{
int i;

    if (!sockfd && (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    for (i = 0; i < MAX_POLL; i++)
        if (!pollstr[i].events)
		break;
    if (i == MAX_POLL) {
        printf ("error no sock\n");
        exit(-1);
    }
    pollstr[i].fd = sockfd;
    pollstr[i].events = POLLIN;
    return i;
}

char resp[] = 
    "HTTP/1.0 200 OK\r\n"
    "Server: hacko/0.6 Python/2.6.1\r\n"
    "Date: Fri, 23 Aug 2013 03:28:48 GMT\r\n"
    "Content-type: application/octet-stream\r\n"
    "Content-Length: %d\r\n"
    "Last-Modified: Sat, 06 Jul 2013 22:43:14 GMT\r\n"
    "\r\n";
char cdata[2041];

int main(int argc, char *argv[])
{
        int tindex, sockopt = 1; /* allow reuse of port number */
        static char *lport;
	if (argc != 2) {
	    printf("tcpweb <portnumber>\n");
	    exit(1);
	}
	memset(cdata, 'A', sizeof(cdata));
        lport = (++argv)[0];                     /**** parse mapping specification */
        tindex = get_fd(0);                      /**** get listening socket */
        memset(&dest_addr[tindex], 0, sizeof(dest_addr[tindex]));
        dest_addr[tindex].sin_family = AF_INET;
        dest_addr[tindex].sin_port = htons(atol(lport));
        dest_addr[tindex].sin_addr.s_addr = htonl(INADDR_ANY);
        if (setsockopt(pollstr[tindex].fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) < 0) 
            perror ("cannot set socket to reuse address\n"); /* not fatal */
        if (bind(pollstr[tindex].fd, (struct sockaddr *)&dest_addr[tindex], sizeof(dest_addr[tindex])) < 0) {
            perror("bind");
            exit(1);
        }
        if (listen(pollstr[tindex].fd, 10) < 0) {
            perror("listen");
            exit(1);
        }
    while (1) {                                   /**** main event loop */
        int npoll, tindex;
        if ((npoll = poll(pollstr, MAX_POLL, POLL_TIMEOUT)) < 0) {
            printf ("poll\n");
            break;
        }
        for (tindex = 0; npoll && tindex < MAX_POLL; tindex++) {
            int read_len;
            if(!pollstr[tindex].revents)
                continue;
            npoll--;
printf("[%s:%d] tindex %d other %d fd %d\n", __FUNCTION__, __LINE__, tindex, otherfd[tindex], pollstr[tindex].fd);
            if (!otherfd[tindex]) {             /**** new connection request arrived */
                int i;
                struct sockaddr_in client;
                unsigned int clientlen = sizeof(client);
                if ((i = get_fd(accept(pollstr[tindex].fd,
                           (struct sockaddr *)&client, &clientlen)))<0){
                    perror("accept");
                    exit(1);
                }
printf("[%s:%d] other %d\n", __FUNCTION__, __LINE__, i);
                otherfd[i] = 1000;
            }
            else {
printf("[%s:%d]\n", __FUNCTION__, __LINE__);
                if ((read_len = read(pollstr[tindex].fd, temp_buf, sizeof(temp_buf))) <= 0) {
                    }
                else {
                    temp_buf[read_len] = 0;
                    //memdump(temp_buf, read_len, "RX");
                    printf("INCOMING: %s\n", temp_buf);
                    sprintf(temp_buf, resp, sizeof(cdata));
                    write(pollstr[tindex].fd, temp_buf, strlen(temp_buf));
                    write(pollstr[tindex].fd, cdata, sizeof(cdata));
                };
                shutdown(pollstr[tindex].fd, SHUT_RDWR);
                close(pollstr[tindex].fd);
                pollstr[tindex].fd = 0;
                pollstr[tindex].events = 0;
                printf("closed %d %d\n", tindex, pollstr[tindex].fd);
                otherfd[tindex] = 0;
            }
        }
    }
    return 0;
}
