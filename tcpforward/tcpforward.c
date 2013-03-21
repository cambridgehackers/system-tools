# Original author John Ankcorn
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <signal.h>

#define MAX_POLL	100
#define MAX_DUMP_STR	10
#define POLL_TIMEOUT	20 /* milliseconds */
static char temp_buf[4096];
static struct pollfd pollstr[MAX_POLL];
static int otherfd[MAX_POLL];
static struct sockaddr_in dest_addr[MAX_POLL];
static char dumpstr[MAX_POLL][MAX_DUMP_STR];
static FILE *logfile;
static int connection_count;

static void sig_handler( int arg_signal )
{
    printf ("received signal, exit\n");
    fclose(logfile);
    exit(-1);
}

static void memdump(unsigned char *p, int len, char *title)
{
int i = 0;

    while (len > 0) {
        if ((i & 0xf) == 0) {
                if (i > 0)
                fprintf(logfile, "\n");
                fprintf(logfile, "%s: ",title);
        }
        fprintf(logfile, "%02x ", *p++);
        i++;
        len--;
    }
    fprintf(logfile, "\n");
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

int main(int argc, char *argv[])
{
    signal(SIGINT, sig_handler);    /* also SIGHUP SIGQUIT SIGALRM SIGPIPE */
    while (--argc) {
        int tindex, sockopt = 1; /* allow reuse of port number */
        static char *lport, *hport, *haddr;
        struct hostent *hp;
        lport = (++argv)[0];                     /**** parse mapping specification */
        if (!(haddr = strchr(lport, ':')) || !(hport = strchr(haddr+1, ':'))) {
            printf ("source local:remip:remport\n");
            exit(-1);
        }
        *haddr++ = 0;
        *hport++ = 0;
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
        dest_addr[tindex].sin_family = AF_INET;   /**** initialize target address parameters */
        dest_addr[tindex].sin_port = htons(atol(hport));
        if ((hp = gethostbyname(haddr)) == NULL) {
            fprintf(stderr, "%s: host unknown.\n", haddr);
            exit(1);
        }
        memcpy(&dest_addr[tindex].sin_addr, hp->h_addr, hp->h_length);
    }
    logfile = fopen("tcpforward.logfile", "w");
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
            if (!otherfd[tindex]) {             /**** new connection request arrived */
                int i, j;
                struct sockaddr_in client;
                int clientlen = sizeof(client);
                if ((i = get_fd(accept(pollstr[tindex].fd,
                           (struct sockaddr *)&client, &clientlen)))<0){
                    perror("accept");
                    exit(1);
                }
                j = get_fd(0);
                if (connect(pollstr[j].fd, (struct sockaddr*) &dest_addr[tindex],
                           sizeof(dest_addr[tindex])) < 0) {
                    perror("connect");
                    exit(1);
                }
                otherfd[i] = pollstr[j].fd;
                otherfd[j] = pollstr[i].fd;
                sprintf(dumpstr[i], "TX%d", connection_count);
                sprintf(dumpstr[j], "RX%d", connection_count);
		connection_count++;
                printf("connecting %s:%d to %s:%d\n", inet_ntoa(client.sin_addr), client.sin_port,
                    inet_ntoa(dest_addr[tindex].sin_addr), dest_addr[tindex].sin_port);
            }
            else {                              /**** new data arrived, forward it */
                if ((read_len = read(pollstr[tindex].fd, temp_buf, sizeof(temp_buf))) <= 0
                  || write(otherfd[tindex], temp_buf, read_len) != read_len) {
                    shutdown(pollstr[tindex].fd, SHUT_RDWR);
                    close(pollstr[tindex].fd);
                    pollstr[tindex].fd = 0;
                    pollstr[tindex].events = 0;
                    printf("closed %d %d\n", tindex, pollstr[tindex].fd);
                    }
                else
                    memdump(temp_buf, read_len, dumpstr[tindex]);
            }
        }
    }
    return 0;
}
