# Copyright 2013, John Ankcorn
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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define FOURCC(a) (((a[0]) << 24) | ((a[1]) << 16) | ((a[2]) << 8) | (a[3]))
static int fd, end_of_file = 0;

static struct {
    char *block_type;
    int recurse;
    int dont_print;
} recurse_type[] = {
    { "ftyp", 0, 0}, { "moov", 1, 0}, { "trak", 1, 0}, { "mdia", 1, 0},
    { "minf", 1, 0}, { "stbl", 1, 0}, { "free", 0, 1}, { "mdat", 0, 1},
    {0, 0}};

static unsigned int readbyte(int len)
{
    unsigned int ret = 0;
    while (len--) {
        ret <<= 8;
        if (read(fd, &ret, 1) != 1)
            end_of_file = 1;
    }
    return ret;
}

static void do_indent(int aindent)
{
    while(aindent-- > 0)
        printf ("  ");
}

static int get_block(int aindent)
{
    int orig_len = readbyte(4);
    if (end_of_file)
        return 0;
    int block_type = readbyte(4);
    int index = 0, linecount = 0, len = orig_len - 8;

    do_indent(aindent);
    printf ("%c%c%c%c [%x]\n", block_type >> 24, (block_type >> 16) & 0xff, (block_type >> 8) & 0xff, block_type & 0xff, orig_len);
    while (recurse_type[index].block_type && FOURCC(recurse_type[index].block_type) != block_type)
        index++;
    if (recurse_type[index].recurse)
        while (len > 0)
            len -= get_block(aindent+1);
    else if (recurse_type[index].dont_print)
        lseek(fd, len, SEEK_CUR);
    else {
        while (len > 0) {
            if (linecount++ % 8 == 0) {
                if (linecount != 1)
                    printf("\n");
                do_indent(aindent+1);
            }
            if (len >= 4) {
                int i = readbyte(4);
                if (block_type == FOURCC("stts")) {
                    printf ("%x:%x ", i, readbyte(4));
                    len -= 4;
                }
                else if (block_type == FOURCC("stsz"))
                    printf ("%5x ", i);
                else
                    printf ("%08x ", i);
                len -= 4;
            }
            else while (len--)
                printf ("%02x ", readbyte(1));
        }
        printf ("\n");
    }
    return orig_len;
}

int main(int argc, char *argv[])
{
    if (argc != 2 || (fd = open (argv[1], O_RDONLY)) < 0) {
        printf ("dumpmp4 <filename>\n");
        exit(-1);
    }
    while (!end_of_file)
        get_block(0);
    return 0;
}
