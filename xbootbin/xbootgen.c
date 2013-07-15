/* Copyright 2013, John Ankcorn
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

/* Attribute word defines */
//ATTRIBUTE_PS_IMAGE_MASK        = 0x10    /**< Code partition */
//ATTRIBUTE_PL_IMAGE_MASK        = 0x20    /**< Bit stream partition */
typedef struct {
    uint32_t Version;
    uint32_t ImageCount;
    uint32_t PartitionOffset;
    uint32_t ImageOffset;
} ImageHeaderTable;
typedef struct {
    uint32_t ImageWordLen;
    uint32_t DataWordLen;
    uint32_t PartitionWordLen;
    uint32_t LoadAddr;
    uint32_t ExecAddr;
    uint32_t PartitionStart;
    uint32_t PartitionAttr;
    uint32_t SectionCount;
    uint32_t Pads[7];
    uint32_t CheckSum;
} BootPartitionHeader;
typedef struct {
    uint32_t next;
    uint32_t partition;
    uint32_t count;
    uint32_t name_length;
    //uint32_t name;
} ImageHeader;

static int fdfsbl, fdcomposite, fdoutfile, end_of_file = 0;
static unsigned char buffer[BUFFER_SIZE];
ImageHeaderTable imagetab = {
    0x1010000, 3, 0x260, 0x240};
BootPartitionHeader partinit = {0xaabbccdd};
BootPartitionHeader zpartinit;

static void memdump(unsigned char *p, int len, char *title)
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
    BootPartitionHeader part_data[20], *ppart = part_data;
    int i;

printf("argc %d a %s a %s\n", argc, argv[1], argv[2]);
    if (argc != 3
     || (fdfsbl = open (argv[1], O_RDONLY)) < 0
     || (fdcomposite = open (argv[2], O_RDONLY)) < 0
     || (fdoutfile = creat ("boot.tmp", 0666)) < 0) {
        printf ("xbootgen <fsbl> <composite>\n");
        exit(-1);
    }
    lseek(fdfsbl, 0, SEEK_SET);
    lseek(fdcomposite, 0, SEEK_SET);
    int tmpfd = open ("d.tmp", O_RDONLY);
    int j;
    int len = read(tmpfd, buffer, sizeof(buffer));
    unsigned char fillbyte = 0xff;
    struct {
        uint32_t address;
        uint32_t value;
    } reginit = {0xffffffff, 0};

    write(fdoutfile, buffer, len);
    for(i = 0; i < 256; i++)
        write(fdoutfile, &reginit, sizeof(reginit));
    i = 32;
    while(i-- > 0)
        write(fdoutfile, &fillbyte, sizeof(fillbyte));

    write(fdoutfile, &imagetab, sizeof(imagetab));
    i = 64 - sizeof(imagetab);
    while(i-- > 0)
        write(fdoutfile, &fillbyte, sizeof(fillbyte));

    int tnext = 0x250;
    for (i = 0; i < 2; i++) {
        ImageHeader imagehead;
        union {
            char c[200];
            uint32_t i[50];
        } nametemp;

        memset(&nametemp, 0, sizeof(nametemp));
        strcpy(nametemp.c, argv[1+i]);
        imagehead.next = tnext;
        tnext = 0;
        imagehead.partition = 0x260 + i * 0x10;
        imagehead.count = 0;
        imagehead.name_length = i + 1; /* value of actual partition count */
        printf("NAMELEN %d\n", imagehead.name_length);
        write(fdoutfile, &imagehead, sizeof(imagehead));
        int wordlen = (strlen(nametemp.c) + 7)/4;
        for (j = 0; j < wordlen; j++) {
            nametemp.i[j] = ntohl(nametemp.i[j]);
            write(fdoutfile, &nametemp.i[j], sizeof(nametemp.i[j]));
        }
        j = 64 - sizeof(imagehead) - wordlen * 4;
        while(j-- > 0)
            write(fdoutfile, &fillbyte, sizeof(fillbyte));
    }
    for (i = 0; i < imagetab.ImageCount; i++) {
        partinit.ImageWordLen = 0x55b9;
        partinit.DataWordLen = 0x55b9;
        partinit.PartitionWordLen = 0x55b9;
        partinit.LoadAddr = 0;
        partinit.ExecAddr = 0;
        partinit.PartitionStart = 0x2a0;
        partinit.PartitionAttr = 0x10;
        partinit.SectionCount = 1;
        partinit.CheckSum = 0xfffef9e3;
        partinit.Pads[1] = 0x240 + i * 0x10;
        write(fdoutfile, &partinit, sizeof(partinit));
    }
    /* last partition entry is all '0' */
    zpartinit.CheckSum = 0xffffffff;
    write(fdoutfile, &zpartinit, sizeof(zpartinit));

    uint32_t totalsize = 0x156e4;
    lseek(fdoutfile, 0x34, SEEK_SET);
    write(fdoutfile, &totalsize, sizeof(totalsize));
    lseek(fdoutfile, 0x40, SEEK_SET);
    write(fdoutfile, &totalsize, sizeof(totalsize));
    close(fdoutfile);
//#define IMAGE_PHDR_OFFSET 0x09C    /* Start of partition headers */
//    lseek(fd, IMAGE_PHDR_OFFSET, SEEK_SET);
//    uint32_t part_offset;
//    read(fd, &part_offset, sizeof(part_offset));
//printf("[%s:%d] off %x\n", __FUNCTION__, __LINE__, part_offset);
//    lseek(fd, part_offset, SEEK_SET);
//    while (!end_of_file) {
//        read(fd, ppart, sizeof(*ppart));
//        if (ppart->CheckSum == 0xffffffff)
//            break;
//        ppart++;
//    }
//    int pindex = 0;
//    while (&part_data[pindex] != ppart) {
//        printf("    ImageWordLen: %8d; ", part_data[pindex].ImageWordLen << 2);
//        printf("DataWordLen: %8d; ", part_data[pindex].DataWordLen << 2);
//        printf("PartitionWordLen: %8d\n", part_data[pindex].PartitionWordLen << 2);
//        printf("        LoadAddr: %8x; ", part_data[pindex].LoadAddr);
//        printf("ExecAddr:    %8x; ", part_data[pindex].ExecAddr);
//        printf("PartitionStart:   %8x\n", part_data[pindex].PartitionStart << 2);
//        printf("        PartitionAttr: %3x; ", part_data[pindex].PartitionAttr);
//        printf("SectionCount: %7x\n", part_data[pindex].SectionCount);
//        lseek(fd, part_data[pindex].PartitionStart << 2, SEEK_SET);
//        int rlen = read(fd, buffer, sizeof(buffer));
//        memdump(buffer, rlen, "DATA");
//        pindex++;
//    }
    return 0;
}
