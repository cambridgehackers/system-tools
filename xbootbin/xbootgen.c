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
#include <endian.h>

#define BUFFER_SIZE 1024

/************ elf header structures ****************/
typedef struct {
    unsigned char e_ident[16];
    uint16_t  e_type;
#define ET_EXEC 2
    uint16_t  e_machine;
    uint32_t  e_version;
    uint32_t  e_entry;
    uint32_t  e_phoff;
    uint32_t  e_shoff;
    uint32_t  e_flags;
    uint16_t  e_ehsize;
    uint16_t  e_phentsize;
    uint16_t  e_phnum;
    uint16_t  e_shentsize;
    uint16_t  e_shnum;
    uint16_t  e_shstrndx;
} elf_header;

enum{PT_NULL, PT_LOAD, PT_DYNAMIC, PT_INTERP, PT_NOTE, PT_SHLIB, PT_PHDR};
typedef struct {
    uint32_t  p_type;
    uint32_t  p_offset;
    uint32_t  p_vaddr;
    uint32_t  p_paddr;
    uint32_t  p_filesz;
    uint32_t  p_memsz;
    uint32_t  p_flags;
    uint32_t  p_align;
} program_header;

/************ boot.bin header structures ****************/
#define ATTRIBUTE_PS_IMAGE_MASK  0x10    /**< Code partition */
#define ATTRIBUTE_PL_IMAGE_MASK  0x20    /**< Bit stream partition */
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
} ImageHeader;

static int fdinput[2], fdoutfile, end_of_file = 0;

static program_header  *progh[2];
static elf_header       elfh[2];
static unsigned char    buffer[BUFFER_SIZE];
static ImageHeaderTable imagetab = {0x1010000};
static BootPartitionHeader partinit[10];

static void fill_file(void)
{
    int filllen = 0x40 - (lseek(fdoutfile, 0, SEEK_CUR) & 0x3f);
    if (filllen == 0x40)
        return;
    static unsigned char fillbyte = 0xff;
    while(filllen-- > 0)
        write(fdoutfile, &fillbyte, sizeof(fillbyte));
}

#define PARTITION_HEADER_TABLE 0x980
static uint32_t boot_rom_header[32] = {
        /* From TRM, Chapter 6: Boot and Configuration */
	0xaa995566,
	0x584c4e58,
	0,
	0x1010000,
};
int main(int argc, char *argv[])
{
    BootPartitionHeader part_data[20], *ppart = part_data;
    int i, index, j, entry;

    if (argc != 3
     || (fdinput[0] = open (argv[1], O_RDONLY)) < 0
     || (fdinput[1] = open (argv[2], O_RDONLY)) < 0
     || (fdoutfile = creat ("boot.tmp", 0666)) < 0) {
        printf ("xbootgen <fsbl> <composite>\n");
        exit(-1);
    }
    int tmpfd = open ("d.tmp", O_RDONLY);
    int len = read(tmpfd, buffer, sizeof(buffer));
    close(tmpfd);
    write(fdoutfile, buffer, len);
    int rom_header_offset = lseek(fdoutfile, 0, SEEK_CUR);
    write(fdoutfile, boot_rom_header, sizeof(boot_rom_header));
    for(i = 0; i < 256; i++) {
        struct {
            uint32_t address;
            uint32_t value;
        } reginit = {0xffffffff, 0};
        write(fdoutfile, &reginit, sizeof(reginit));
    }
    fill_file();

    int partstart = 0x2a0;
    for (index = 0; index < 2; index++) {
        int startsect = imagetab.ImageCount;
        if(read(fdinput[index], &(elfh[index]), sizeof(elfh[index])) != sizeof(elfh[index])) {
            printf("[%s:%d] error in read\n", __FUNCTION__, __LINE__);
        }
        if (elfh[index].e_ident[0] != 0x7f || elfh[index].e_ident[1] != 'E' || elfh[index].e_ident[2] != 'L'
         || elfh[index].e_ident[3] != 'F' || elfh[index].e_ident[6] != 1
         || elfh[index].e_type != ET_EXEC || elfh[index].e_ident[4] != 1) {
            printf("Error: input file not valid\n");
        }
        if (elfh[index].e_phnum != 0) {
            if (lseek(fdinput[index], elfh[index].e_phoff, SEEK_SET) == -1) {
                fprintf(stderr, "Error seeking to offset\n");
                return 1;
            }
            progh[index] = malloc(elfh[index].e_phentsize * elfh[index].e_phnum);
            if(read(fdinput[index], progh[index], elfh[index].e_phentsize * elfh[index].e_phnum) != (elfh[index].e_phentsize * elfh[index].e_phnum)) {
                printf("[%s:%d] error in read\n", __FUNCTION__, __LINE__);
            }
            uint32_t enaddr = elfh[index].e_entry;
            for (entry = 0; entry < elfh[index].e_phnum; ++entry)
                if (progh[index][entry].p_filesz) {
                    uint32_t datalen = progh[index][entry].p_filesz;
                    if (index == 0) {
                        boot_rom_header[5] = datalen;
                        boot_rom_header[8] = datalen;
                    }
                    datalen /= 4;
                    partinit[imagetab.ImageCount].ImageWordLen = datalen;
                    partinit[imagetab.ImageCount].DataWordLen = datalen;
                    partinit[imagetab.ImageCount].PartitionWordLen = datalen;
                    partinit[imagetab.ImageCount].LoadAddr = progh[index][entry].p_paddr;
                    partinit[imagetab.ImageCount].ExecAddr = enaddr;
                    partinit[imagetab.ImageCount].PartitionStart = partstart;
                    partinit[imagetab.ImageCount].PartitionAttr = ATTRIBUTE_PS_IMAGE_MASK;
                    partinit[imagetab.ImageCount].SectionCount = 0;
                    partinit[imagetab.ImageCount].Pads[1] = startsect * sizeof(partinit[0])/4;
                    partinit[startsect].SectionCount++;
                    imagetab.ImageCount++;
                    enaddr = 0;
                    int fsize = datalen & 63;
                    if (fsize)
                        datalen += (64 - fsize);
                    partstart += datalen;
                }
        }
    }
    int toff;
    int imagetab_offset = lseek(fdoutfile, 0, SEEK_CUR);
    boot_rom_header[30] = imagetab_offset;
    write(fdoutfile, &imagetab, sizeof(imagetab));
    fill_file();

    imagetab.ImageOffset = lseek(fdoutfile, 0, SEEK_CUR)/4;
    int tnext = lseek(fdoutfile, 0, SEEK_CUR);
    ImageHeader imagehead[2];
    for (index = 0; index < 2; index++) {
        union {
            char c[200];
            uint32_t i[50];
        } nametemp;

        memset(&nametemp, 0, sizeof(nametemp));
        strcpy(nametemp.c, argv[1+index]);
        int wordlen = (strlen(nametemp.c) + 7)/4;
        tnext += sizeof(imagehead[index]) + wordlen * 4 + (64 - sizeof(imagehead[index]) - wordlen * 4);
        if (index != 0)
            tnext = 0;
        imagehead[index].next = tnext/4;
printf("[%s:%d] tnext %x\n", __FUNCTION__, __LINE__, tnext/4);
        imagehead[index].partition = 0x260 + index * sizeof(partinit[0])/4;
        imagehead[index].count = 0;
        imagehead[index].name_length = index + 1; /* value of actual partition count */
toff = lseek(fdoutfile, 0, SEEK_CUR)/4;
printf("[%s:%d] 240/250 toff %x\n", __FUNCTION__, __LINE__, toff);
        write(fdoutfile, &imagehead[index], sizeof(imagehead[index]));
        for (j = 0; j < wordlen; j++) {
            nametemp.i[j] = ntohl(nametemp.i[j]);
            write(fdoutfile, &nametemp.i[j], sizeof(nametemp.i[j]));
        }
        fill_file();
    }

    /* last partition entry is all '0' */
    for (i = 0; i <= imagetab.ImageCount; i++) {
        if (i != imagetab.ImageCount)
            partinit[i].Pads[1] += imagetab.ImageOffset;
        uint32_t checksum = 0, *pdata = (uint32_t *)&partinit[i];
        for (j = 0; j < 15; j++)
             checksum += *pdata++;
        partinit[i].CheckSum = ~checksum;
    }
    boot_rom_header[31] = lseek(fdoutfile, 0, SEEK_CUR);
printf("[%s:%d] 260 toff %x\n", __FUNCTION__, __LINE__, boot_rom_header[31]);
    imagetab.PartitionOffset = boot_rom_header[31]/4;
    write(fdoutfile, &partinit, sizeof(partinit[0]) * (imagetab.ImageCount+1));
    boot_rom_header[4] = lseek(fdoutfile, 0, SEEK_CUR);
    for (index = 0; index < 2; index++)
        if (progh[index]) {
            fill_file();
            for (entry = 0; entry < elfh[index].e_phnum; ++entry)
                if (progh[index][entry].p_filesz) {
                    lseek(fdinput[index], progh[index][entry].p_offset, SEEK_SET);
                    uint32_t readlen = progh[index][entry].p_filesz;
    toff = lseek(fdoutfile, 0, SEEK_CUR);
printf("[%s:%d] 2a0/etc datttt toff %x\n", __FUNCTION__, __LINE__, toff/4);
                    while (readlen > 0) {
                        int readitem = readlen;
                        if (readitem > sizeof(buffer))
                            readitem = sizeof(buffer);
                        int len = read(fdinput[index], buffer, readitem);
                        if (len != readitem)
                            printf("tried to read file %d length %d actual %d\n", index, readitem, len);
                        write(fdoutfile, buffer, len);
                        readlen -= len;
                    }
                }
        }
    boot_rom_header[9] = 1;
    for (index = 0; index < 10; index++)
         boot_rom_header[10] += boot_rom_header[index];
    boot_rom_header[10] = ~boot_rom_header[10];
    lseek(fdoutfile, imagetab_offset, SEEK_SET);
    write(fdoutfile, &imagetab, sizeof(imagetab));
    lseek(fdoutfile, rom_header_offset, SEEK_SET);
    write(fdoutfile, boot_rom_header, sizeof(boot_rom_header));
    close(fdoutfile);
    return 0;
}
