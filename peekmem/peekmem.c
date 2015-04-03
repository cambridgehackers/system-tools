// Copyright (c) 2015 Quanta Research Cambridge, Inc.

// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>

#define MAP_LENGTH  0x1000000
#define MAP_BASE   0xf8000000

static void memdump(uint8_t *p, int len, const char *title)
{
int i;

    i = 0;
    while (len > 0) {
        if (!(i & 0xf)) {
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


int main(int argc, char *argv[])
{
printf("[%s:%d] start\n", __FUNCTION__, __LINE__);
    int fd = open("/dev/mem", O_RDONLY|O_SYNC);
    if (fd == -1) {
        printf("[%s:%d] openfail\n", __FUNCTION__, __LINE__);
        exit(-1);
    }
    uint8_t *mapbase = mmap(NULL, MAP_LENGTH, PROT_READ, MAP_SHARED, fd, MAP_BASE);
    if (mapbase == MAP_FAILED) {
        printf("[%s:%d] mmap fail\n", __FUNCTION__, __LINE__);
        exit(-1);
    }

    argc--;
    argv++;
    while (argc-- > 0) {
        int len = 1;
        char *pend;
printf("[%s:%d] %s\n", __FUNCTION__, __LINE__, *argv);
        unsigned long long pbase = strtoll(*argv++, &pend, 16);
        if (argc-- > 0)
            len = strtol(*argv++, &pend, 16);
printf("[%s:%d] %8llx %8x\n", __FUNCTION__, __LINE__, pbase, len);
        memdump(mapbase+pbase, len, "XX");
    }
    return 0;
}
