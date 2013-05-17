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
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>

#define IMAGE_PHDR_OFFSET 0x09C    /* Start of partition headers */

/* Attribute word defines */
//ATTRIBUTE_PS_IMAGE_MASK        = 0x10    /**< Code partition */
//ATTRIBUTE_PL_IMAGE_MASK        = 0x20    /**< Bit stream partition */
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

static int get_block(int aindent)
{
    int orig_len = readbyte(4);
    if (end_of_file)
        return 0;
    int block_type = readbyte(4);
    int index = 0, linecount = 0, len = orig_len - 8;

    return orig_len;
}

int main(int argc, char *argv[])
{
    BootPartitionHeader part_data;

    if (argc != 2 || (fd = open (argv[1], O_RDONLY)) < 0) {
        printf ("dumpmp4 <filename>\n");
        exit(-1);
    }
    lseek(fd, IMAGE_PHDR_OFFSET, SEEK_SET);
    uint32_t part_offset;
    read(fd, &part_offset, sizeof(part_offset));
printf("[%s:%d] off %x\n", __FUNCTION__, __LINE__, part_offset);
    lseek(fd, part_offset, SEEK_SET);
    while (!end_of_file) {
        read(fd, &part_data, sizeof(part_data));
        if (part_data.ImageWordLen == 0)
            break;
        printf("    ImageWordLen: %8x; ", part_data.ImageWordLen);
        printf("DataWordLen: %8x; ", part_data.DataWordLen);
        printf("PartitionWordLen: %8x\n", part_data.PartitionWordLen);
        printf("        LoadAddr: %8x; ", part_data.LoadAddr);
        printf("ExecAddr:    %8x; ", part_data.ExecAddr);
        printf("PartitionStart:   %8x\n", part_data.PartitionStart);
        printf("        PartitionAttr: %3x; ", part_data.PartitionAttr);
        printf("SectionCount: %7x\n", part_data.SectionCount);
        //get_block(0);
    }
    return 0;
}
