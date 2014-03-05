// Copyright (c) 2014 Quanta Research Cambridge, Inc.

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

//
// FTDI interface documented at:
// http://www.ftdichip.com/Documents/AppNotes/AN2232C-01_MPSSE_Cmnd.pdf

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ftdi_reference.h"

#define IDCODE_VALUE 0x93, 0x10, 0x65, 0x43

#define MREAD    (MPSSE_LSB|MPSSE_READ_NEG)
#define MWRITE   (MPSSE_LSB|MPSSE_WRITE_NEG)

#define TMSW     (MPSSE_WRITE_TMS               |MWRITE|MPSSE_BITMODE) // 4b
#define TMSR     (MPSSE_WRITE_TMS|MPSSE_DO_READ |MREAD|MWRITE|MPSSE_BITMODE) // 6f
#define DATARW(A)  (MPSSE_DO_READ|MPSSE_DO_WRITE|MREAD|MWRITE), /* 3d */ \
    (((A)-1) & 0xff), (((A)-1) >> 8)
      
#define DATAR      (MPSSE_DO_READ               |MREAD) // 2c
#define DATAWBIT   (MPSSE_DO_WRITE              |MWRITE|MPSSE_BITMODE) // 1b
#define DATARBIT   (MPSSE_DO_READ               |MREAD|MPSSE_BITMODE) // 2e
#define DATARW_BIT (MPSSE_DO_READ|MPSSE_DO_WRITE|MREAD|MWRITE|MPSSE_BITMODE) // 3f
#define DATAW_BYTES 0x19
#define DATAW_BYTES_LEN(A) 0x19, \
    (((A)-1) & 0xff), (((A)-1) >> 8)

static unsigned char item14z[] = { TMSW, 0x02, 0x07, };
static unsigned char errorcode_fa[] = { 0xfa, };
static unsigned char readdata8z[] = { 0x02, 0x08, 0x9e, 0x7f, 0x0f, };
#define BUFFER_MAX_LEN 1000000
static struct ftdi_transfer_control* writetc;
static int inputfd;

static void check_ftdi_read_data_submit(struct ftdi_context *ftdi, unsigned char *buf, int size)
{
    struct ftdi_transfer_control* tc = ftdi_read_data_submit(ftdi, buf, size);
    ftdi_transfer_data_done(writetc);
    ftdi_transfer_data_done(tc);
    //printf("[%s:%d]\n", __FUNCTION__, __LINE__);
}
static void test_pattern(struct ftdi_context *ftdi)
{
static unsigned char item5z[] = {
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0xff, 
     TMSW, 0x02, 0x83, 
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0xc3, 
     TMSW, 0x02, 0x03, 
     TMSW, 0x02, 0x01, 
     DATAR, 0x02, 0x00, 
     DATARBIT, 0x06, 
     TMSR, 0x02, 0x03, 
     0x87, 
};
static unsigned char item6z[] = {
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0xff, 
     TMSW, 0x02, 0x83, 
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0xc3, 
     TMSW, 0x02, 0x03, 
     TMSW, 0x02, 0x01, 
     DATAW_BYTES_LEN(1), 0x69, 
     DATAWBIT, 0x01, 0x00, 
     DATAR, 0x02, 0x00, 
     DATARBIT, 0x06, 
     TMSR, 0x02, 0x03, 
     0x87, 
};
static unsigned char item7z[] = {
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0xff, 
     TMSW, 0x02, 0x83, 
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0xc3, 
     TMSW, 0x02, 0x03, 
     TMSW, 0x02, 0x01, 
     DATAWBIT, 0x04, 0x0c, 
     TMSW, 0x02, 0x03, 
     TMSW, 0x02, 0x01, 
     DATAW_BYTES_LEN(1), 0x69, 
     DATAWBIT, 0x01, 0x00, 
     DATAR, 0x02, 0x00, 
     DATARBIT, 0x06, 
     TMSR, 0x02, 0x03, 
     0x87, 
};
static unsigned char readdata_five_zeros[] = { 0x00, 0x00, 0x00, 0x00, 0x00, };
int i;
    writetc = ftdi_write_data_submit(ftdi, item5z, sizeof(item5z));
    check_ftdi_read_data_submit(ftdi, readdata_five_zeros, sizeof(readdata_five_zeros));
    writetc = ftdi_write_data_submit(ftdi, item6z, sizeof(item6z));
    check_ftdi_read_data_submit(ftdi, readdata_five_zeros, sizeof(readdata_five_zeros));
    for (i = 0; i < 2; i++) {
        writetc = ftdi_write_data_submit(ftdi, item7z, sizeof(item7z));
        check_ftdi_read_data_submit(ftdi, readdata_five_zeros, sizeof(readdata_five_zeros));
    }
}

#define IDTEST_PATTERN \
     TMSW, 0x04, 0x7f,  \
     TMSW, 0x03, 0x02,  \
     DATARW(63), \
         0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, \
         0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, \
         0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, \
         0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, \
         0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, \
         0x00, 0xff, 0x00, 0x00, 0x00, \
         0xff, 0x00, 0x00,  \
     DATARW_BIT, 0x06, 0x00,  \
     TMSR, 0x02, 0x03,  \
     0x87, 
static unsigned char item3z[] = { TMSW, 0x00, 0x01, IDTEST_PATTERN };
static unsigned char item4z[] = { TMSW, 0x02, 0x07, IDTEST_PATTERN };
static unsigned char readdata3z[] = {
     IDCODE_VALUE,
         0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
         0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
         0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff,
         0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
         0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
         0x00, 0xff, 0x00, 0x00, 0x00,
         0x00, 
};
static void test_different(struct ftdi_context *ftdi)
{
    int j;
    writetc = ftdi_write_data_submit(ftdi, item4z, sizeof(item4z));
    check_ftdi_read_data_submit(ftdi, readdata3z, sizeof(readdata3z)); // IDCODE 00ff
    for (j = 0; j < 3; j++)
        test_pattern(ftdi);
}
#define FILE_READSIZE 6464
int main()
{
    unsigned char bitswap[256];
    struct ftdi_context *ctxitem0z;
    int i, j, k;
    struct ftdi_device_list *devlist, *curdev;
    char tempstr0z[64];
    char tempstr1z[128];
    char tempstr2z[64];

    ctxitem0z = ftdi_new();
    ftdi_init(ctxitem0z);
    ftdi_usb_find_all(ctxitem0z, &devlist, 0x0, 0x0);
    curdev = devlist;
    ftdi_usb_get_strings(ctxitem0z, curdev->dev, tempstr2z, sizeof(tempstr2z), tempstr1z, sizeof(tempstr1z), tempstr0z, sizeof(tempstr0z));

    printf("[%s:%d] %s %s %s\n", __FUNCTION__, __LINE__, tempstr2z, tempstr1z, tempstr0z);
    ftdi_usb_open_dev(ctxitem0z, curdev->dev);
    ftdi_usb_reset(ctxitem0z);
    ftdi_list_free(&devlist);

    int eeprom_val;
    unsigned char fbuf[256]; // since chiptype is 0x56, eerom size is 256
    ftdi_read_eeprom(ctxitem0z);
    ftdi_eeprom_decode(ctxitem0z, 0);
    ftdi_get_eeprom_value(ctxitem0z, CHIP_TYPE, &eeprom_val);
    printf("[%s:%d] CHIP_TYPE %x\n", __FUNCTION__, __LINE__, eeprom_val);
    ftdi_get_eeprom_buf(ctxitem0z, fbuf, sizeof(fbuf));

    ftdi_set_baudrate(ctxitem0z, 9600);
    ftdi_set_latency_timer(ctxitem0z, 255);
    ftdi_set_bitmode(ctxitem0z, 0, 0);
    ftdi_set_bitmode(ctxitem0z, 0, 2);
    ftdi_usb_purge_buffers(ctxitem0z);
    ftdi_usb_purge_rx_buffer(ctxitem0z);
    ftdi_usb_purge_tx_buffer(ctxitem0z);

    for (i = 0; i < 4; i++) {
static unsigned char command_aa[] = { 0xaa, 0x87, };
        ftdi_write_data(ctxitem0z, command_aa, sizeof(command_aa));
        ftdi_read_data(ctxitem0z, errorcode_fa, sizeof(errorcode_fa));
static unsigned char readdata1z[] = { 0xaa, };
        ftdi_read_data(ctxitem0z, readdata1z, sizeof(readdata1z));
    }
static unsigned char command_ab[] = { 0xab, 0x87, };
    ftdi_write_data(ctxitem0z, command_ab, sizeof(command_ab));
    ftdi_read_data(ctxitem0z, errorcode_fa, sizeof(errorcode_fa));
static unsigned char readdata2z[] = { 0xab, };
    ftdi_read_data(ctxitem0z, readdata2z, sizeof(readdata2z));

static unsigned char initialize_sequence[] = {
     0x85, 
     0x8a, 
     0x86, 0x01, 0x00, 
     0x80, 0xe8, 0xeb, 
     0x82, 0x20, 0x30, 
     0x82, 0x30, 0x00, 
     0x82, 0x00, 0x00, 
     TMSW, 0x04, 0x1f, 
};
    ftdi_write_data(ctxitem0z, initialize_sequence, sizeof(initialize_sequence));
    writetc = ftdi_write_data_submit(ctxitem0z, item3z, sizeof(item3z));
    check_ftdi_read_data_submit(ctxitem0z, readdata3z, sizeof(readdata3z)); // IDCODE 00ff

    for (k = 0; k < 2; k++)
        test_different(ctxitem0z);

static unsigned char item8z[] = {
     TMSW, 0x02, 0x07, 
     TMSW, 0x00, 0x7f, 
};
    writetc = ftdi_write_data_submit(ctxitem0z, item8z, sizeof(item8z));
    ftdi_transfer_data_done(writetc);
static unsigned char command_86[] = { 0x86, 0x01, 0x00, };
    ftdi_write_data(ctxitem0z, command_86, sizeof(command_86));

static unsigned char item10z[] = {
     TMSW, 0x00, 0x01, 
     TMSW, 0x00, 0x7f, 
     TMSW, 0x00, 0x01, 
     TMSW, 0x00, 0x7f, 
     TMSW, 0x00, 0x00, 
     TMSW, 0x02, 0x01, 
     DATARW(0x7f),
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 
     DATARW_BIT, 0x06, 0xff, 
     TMSR, 0x01, 0x81, 
     0x87, 
};
    writetc = ftdi_write_data_submit(ctxitem0z, item10z, sizeof(item10z));
static unsigned char readdata5z[] = {
     IDCODE_VALUE,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 
};
    check_ftdi_read_data_submit(ctxitem0z, readdata5z, sizeof(readdata5z)); // IDCODE ffff

static unsigned char item11z[] = {
     TMSW, 0x04, 0x1f, 
     TMSW, 0x00, 0x7f, 
     TMSW, 0x00, 0x00, 
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0xc8, 
     TMSW, 0x02, 0x03, 
     TMSW, 0x02, 0x01, 
     DATAR, 0x02, 0x00, 
     DATARBIT, 0x06, 
     TMSR, 0x02, 0x03, 
     0x87, 
};
static unsigned char readdata_five_ff[] = { 0xff, 0xff, 0xff, 0xff, 0xff, };
    writetc = ftdi_write_data_submit(ctxitem0z, item11z, sizeof(item11z));
    check_ftdi_read_data_submit(ctxitem0z, readdata_five_ff, sizeof(readdata_five_ff));
    for (i = 0; i < 3; i++) {
static unsigned char item12z[] = {
     TMSW, 0x03, 0x03, 
     DATARW_BIT, 0x04, 0xff, 
     TMSR, 0x02, 0x83, 
     0x87, 
};
static unsigned char readdata7z[] = { 0xaf, 0xf5, };
        writetc = ftdi_write_data_submit(ctxitem0z, item12z, sizeof(item12z));
        check_ftdi_read_data_submit(ctxitem0z, readdata7z, sizeof(readdata7z));
    }

static unsigned char item13z[] = {
     TMSW, 0x02, 0x07, 
     TMSW, 0x00, 0x00, 
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0xc5, 
     TMSW, 0x02, 0x03, 
     TMSW, 0x02, 0x01, 
     DATAW_BYTES_LEN(19),
          0xff, 0xff, 0xff, 0xff, 0x55, 0x99, 0xaa, 0x66, 0x02, 0x00, 0x00,
          0x00, 0x14, 0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 
     DATAWBIT, 0x06, 0x00, 
     TMSW, 0x02, 0x03, 
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0xc4, 
     TMSW, 0x02, 0x03, 
     TMSW, 0x02, 0x01, 
     DATAR, 0x02, 0x00, 
     DATARBIT, 0x06, 
     TMSR, 0x02, 0x03, 
     0x87, 
};
    writetc = ftdi_write_data_submit(ctxitem0z, item13z, sizeof(item13z));
    check_ftdi_read_data_submit(ctxitem0z, readdata8z, sizeof(readdata8z));
    writetc = ftdi_write_data_submit(ctxitem0z, item14z, sizeof(item14z));
    ftdi_transfer_data_done(writetc);
    writetc = ftdi_write_data_submit(ctxitem0z, item3z, sizeof(item3z));
    check_ftdi_read_data_submit(ctxitem0z, readdata3z, sizeof(readdata3z)); // IDCODE 00ff

    for (j = 0; j < 3; j++)
        test_pattern(ctxitem0z);
    for (k = 0; k < 3; k++)
        test_different(ctxitem0z);

static unsigned char item15z[] = {
     TMSW, 0x02, 0x07, 
     TMSW, 0x00, 0x7f, 
     TMSW, 0x00, 0x00, 
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0x0b, 
     TMSW, 0x00, 0x01, 
     TMSW, 0x01, 0x01, 
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0x14, 
     TMSW, 0x00, 0x01, 
     TMSW, 0x01, 0x01, 
     0x80, 0xe0, 0xfb, 
     0x80, 0xe0, 0xfa, 
     0x8f, 0xff, 0xff, 
     0x8f, 0xff, 0xff, 
     0x8f, 0x6b, 0xdc, 
     0x80, 0xe0, 0xfb, 
     0x80, 0xe0, 0xeb, 
     TMSW, 0x03, 0x03, 
     DATARW_BIT, 0x04, 0x14, 
     TMSR, 0x00, 0x01, 
     0x87, 
};
static unsigned char readdata9z[] = { 0x88, 0x44, };
    writetc = ftdi_write_data_submit(ctxitem0z, item15z, sizeof(item15z));
    check_ftdi_read_data_submit(ctxitem0z, readdata9z, sizeof(readdata9z));

static unsigned char item16z[] = {
     TMSW, 0x01, 0x01, 
     TMSW, 0x03, 0x03, 
     DATARW_BIT, 0x04, 0x05, 
     TMSR, 0x00, 0x01, 
     0x87, 
};
static unsigned char readdata10z[] = { 0x8a, 0x45, };
    writetc = ftdi_write_data_submit(ctxitem0z, item16z, sizeof(item16z));
    check_ftdi_read_data_submit(ctxitem0z, readdata10z, sizeof(readdata10z));

    for (i = 0; i < sizeof(bitswap); i++)
        bitswap[i] = ((i &    1) << 7) | ((i &    2) << 5)
           | ((i &    4) << 3) | ((i &    8) << 1)
           | ((i & 0x10) >> 1) | ((i & 0x20) >> 3)
           | ((i & 0x40) >> 5) | ((i & 0x80) >> 7);
    printf("Starting to send file\n");
    inputfd = open("mkPcieTop.bin", O_RDONLY);
    int limit_len = 4032;
    int last = 0;
    static unsigned char hdr1[] = {
          TMSW, 0x01, 0x01, 
          TMSW, 0x02, 0x01, 
          DATAW_BYTES_LEN(4), 0x00, 0x00, 0x00, 0x00, 
    };
    static unsigned char readbuffer[BUFFER_MAX_LEN];
    unsigned char *readptr = readbuffer;
    memcpy(readptr, hdr1, sizeof(hdr1));
    readptr += sizeof(hdr1);
    while (!last) {
        static unsigned char filebuffer[10000];
        unsigned char *ptrin = filebuffer;
    
        int remaining = read(inputfd, filebuffer, FILE_READSIZE);
        last = (remaining < FILE_READSIZE);
        for (i = 0; i < remaining; i++)
            filebuffer[i] = bitswap[filebuffer[i]];
        remaining--;
        while (remaining > 0) {
            int rlen = remaining;
            if (rlen > limit_len)
                rlen = limit_len;
            if (last || rlen < limit_len)
                rlen--;                   // last byte is actually loaded with DATAW command
            *readptr++ = DATAW_BYTES;
            *readptr++ = rlen;
            *readptr++ = rlen >> 8;
            for (i = 0; i < rlen+1; i++)
                *readptr++ = *ptrin++;
            if (last || rlen+1 < limit_len) {
                unsigned char ch = *ptrin++;
                *readptr++ = DATAWBIT;
                *readptr++ = 0x06;
                *readptr++ = ch;        // 7 bits of data here
                if (last) {
                    *readptr++ = TMSW;
                    *readptr++ = 0x00;
                    *readptr++ = 0x01;
                }
                *readptr++ = TMSW;
                *readptr++ = 0x01;
                *readptr++ = 0x01 | (0x80 & ch); // 1 bit if data here
            }
            //printf("[%s:%d] len %ld\n", __FUNCTION__, __LINE__, readptr - readbuffer);
            writetc = ftdi_write_data_submit(ctxitem0z, readbuffer, readptr - readbuffer);
            ftdi_transfer_data_done(writetc);
            writetc = NULL;
            remaining -= limit_len+1;
            if (last)
                break;
            readptr = readbuffer;
        }
        limit_len = 4045;
        *readptr++ = TMSW;
        *readptr++ = 0x01;
        *readptr++ = 0x01;
    }
    printf("[%s:%d] done sending file\n", __FUNCTION__, __LINE__);

#define SYNC_PATTERN(A,B) \
     TMSW, 0x03, 0x03,  \
     DATAWBIT, 0x04, 0x05,  \
     TMSW, 0x00, 0x01,  \
     TMSW, 0x01, 0x01,  \
     TMSW, 0x02, 0x01,  \
     DATAW_BYTES_LEN(4), 0xff, 0xff, 0xff, 0xff,  \
     DATAW_BYTES_LEN(4), 0x55, 0x99, 0xaa, 0x66,  \
     DATAW_BYTES_LEN(4), 0x04, 0x00, 0x00, 0x00,  \
     DATAW_BYTES_LEN(4), 0x14,  (A),  (B), 0x80,  \
     DATAW_BYTES_LEN(4), 0x04, 0x00, 0x00, 0x00,  \
     DATAW_BYTES_LEN(4), 0x04, 0x00, 0x00, 0x00,  \
     DATAW_BYTES_LEN(4), 0x0c, 0x00, 0x01, 0x80,  \
     DATAW_BYTES_LEN(4), 0x00, 0x00, 0x00, 0xb0,  \
     DATAW_BYTES_LEN(4), 0x04, 0x00, 0x00, 0x00,  \
     DATAW_BYTES_LEN(3), 0x04, 0x00, 0x00,  \
     DATAWBIT, 0x06, 0x00,  \
     TMSW, 0x00, 0x01,  \
     TMSW, 0x01, 0x01,  \
     TMSW, 0x03, 0x03,  \
     DATAWBIT, 0x04, 0x04,  \
     TMSW, 0x00, 0x01,  \
     TMSW, 0x01, 0x01,  \
     TMSW, 0x02, 0x01,  \
     DATARW(3), 0x00, 0x00, 0x00,  \
     DATARW_BIT, 0x06, 0x00,  \
     TMSR, 0x00, 0x01,  \
     0x87, 
static unsigned char item17z[] = {
     0x80, 0xe0, 0xfb, 
     0x80, 0xe0, 0xfa, 
     0x8f, 0x3d, 0x49, 
     0x80, 0xe0, 0xfb, 
     0x80, 0xe0, 0xeb, 
     SYNC_PATTERN(0x40, 0x03)
};
static unsigned char readdata11z[] = { 0x00, 0x00, 0x00, 0x00, 0x80, };
    writetc = ftdi_write_data_submit(ctxitem0z, item17z, sizeof(item17z));
    check_ftdi_read_data_submit(ctxitem0z, readdata11z, sizeof(readdata11z));

static unsigned char item18z[] = {
     TMSW, 0x01, 0x01, 
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0x3f, 
     TMSW, 0x00, 0x81, 
     TMSW, 0x01, 0x01, 
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0x0c, 
     TMSW, 0x00, 0x01, 
     TMSW, 0x01, 0x01, 
     TMSW, 0x00, 0x00, 
     TMSW, 0x06, 0x00, 
     TMSW, 0x06, 0x00, 
     TMSW, 0x06, 0x00, 
     TMSW, 0x06, 0x00, 
     TMSW, 0x06, 0x00, 
     TMSW, 0x06, 0x00, 
     TMSW, 0x06, 0x00, 
     TMSW, 0x06, 0x00, 
     TMSW, 0x06, 0x00, 
     TMSW, 0x06, 0x00, 
     TMSW, 0x06, 0x00, 
     TMSW, 0x06, 0x00, 
     TMSW, 0x06, 0x00, 
     TMSW, 0x06, 0x00, 
     TMSW, 0x01, 0x00, 
     TMSW, 0x03, 0x03, 
     DATARW_BIT, 0x04, 0x3f, 
     TMSR, 0x00, 0x81, 
     0x87, 
};
static unsigned char readdata12z[] = { 0xac, 0xd6, };
    writetc = ftdi_write_data_submit(ctxitem0z, item18z, sizeof(item18z));
    check_ftdi_read_data_submit(ctxitem0z, readdata12z, sizeof(readdata12z));

static unsigned char item19z[] = {
     TMSW, 0x01, 0x01, 
     SYNC_PATTERN(0x00, 0x07)
};
static unsigned char readdata13z[] = { 0x02, 0x08, 0x9e, 0x7f, 0x3f, };
    writetc = ftdi_write_data_submit(ctxitem0z, item19z, sizeof(item19z));
    check_ftdi_read_data_submit(ctxitem0z, readdata13z, sizeof(readdata13z));

static unsigned char item20z[] = {
     TMSW, 0x01, 0x01, 
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0x3f, 
     TMSW, 0x00, 0x81, 
     TMSW, 0x01, 0x01, 
};
    writetc = ftdi_write_data_submit(ctxitem0z, item20z, sizeof(item20z));
    ftdi_transfer_data_done(writetc);

static unsigned char item21z[] = {
     TMSW, 0x02, 0x07, 
     TMSW, 0x00, 0x7f, 
     TMSW, 0x00, 0x00, 
     TMSW, 0x03, 0x03, 
     DATARW_BIT, 0x04, 0xff, 
     TMSR, 0x02, 0x83, 
     0x87, 
};
static unsigned char readdata14z[] = { 0xa9, 0xf5, };
    writetc = ftdi_write_data_submit(ctxitem0z, item21z, sizeof(item21z));
    check_ftdi_read_data_submit(ctxitem0z, readdata14z, sizeof(readdata14z));
    test_different(ctxitem0z);

static unsigned char item22z[] = {
     TMSW, 0x02, 0x07, 
     TMSW, 0x00, 0x7f, 
     TMSW, 0x00, 0x01, 
     TMSW, 0x00, 0x00, 
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0xc5, 
     TMSW, 0x02, 0x03, 
     TMSW, 0x02, 0x01, 
     DATAW_BYTES_LEN(19),
          0xff, 0xff, 0xff, 0xff, 0x55, 0x99, 0xaa, 0x66, 0x02, 0x00, 0x00,
          0x00, 0x14, 0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 
     DATAWBIT, 0x06, 0x00, 
     TMSW, 0x02, 0x03, 
     TMSW, 0x03, 0x03, 
     DATAWBIT, 0x04, 0xc4, 
     TMSW, 0x02, 0x03, 
     TMSW, 0x02, 0x01, 
     DATAR, 0x02, 0x00, 
     DATARBIT, 0x06, 
     TMSR, 0x02, 0x03, 
     0x87, 
};
    writetc = ftdi_write_data_submit(ctxitem0z, item22z, sizeof(item22z));
    check_ftdi_read_data_submit(ctxitem0z, readdata8z, sizeof(readdata8z));

    writetc = ftdi_write_data_submit(ctxitem0z, item14z, sizeof(item14z));
    ftdi_transfer_data_done(writetc);
    ftdi_deinit(ctxitem0z);
    return 0;
}
