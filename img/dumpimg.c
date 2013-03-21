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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "bootimg.h"

#define PAD_SIZE 4096
static int fd, end_of_file = 0;

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

void memdump(unsigned char *p, int len, char *title)
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

unsigned char buffer[256];
int main(int argc, char *argv[])
{
    struct boot_img_hdr hdr;
    if (argc != 2 || (fd = open (argv[1], O_RDONLY)) < 0) {
        printf ("dumpimg <filename>\n");
        exit(-1);
    }
    read(fd, &hdr, sizeof(hdr));
    memdump(BOOT_MAGIC, sizeof(BOOT_MAGIC), "origmagic");
    memdump(hdr.magic, sizeof(hdr.magic), "magic");
    if (memcmp(hdr.magic, BOOT_MAGIC, strlen(BOOT_MAGIC))) {
        printf ("Not a boot image file\n");
        exit(-1);
    }
    printf ("kernel_size %x\n", hdr.kernel_size);
    printf ("kernel_addr %x\n", hdr.kernel_addr);
    printf ("ramdisk_size %x\n", hdr.ramdisk_size);
    printf ("ramdisk_addr %x\n", hdr.ramdisk_addr);
    printf ("second_size %x\n", hdr.second_size);
    printf ("second_addr %x\n", hdr.second_addr);
    printf ("tags_addr %x\n", hdr.tags_addr);
    printf ("page_size %x\n", hdr.page_size);
    memdump(hdr.name, sizeof(hdr.name), "name");
    //memdump(hdr.cmdline, sizeof(hdr.cmdline), "cmdline");
    memdump((unsigned char *)hdr.id, sizeof(hdr.id), "id");
    lseek(fd, PAD_SIZE, SEEK_SET);
    read(fd, buffer, sizeof(buffer));
    memdump(buffer, sizeof(buffer), "BUFF");
    return 0;
}
