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
//     http://www.ftdichip.com/Documents/AppNotes/AN2232C-01_MPSSE_Cmnd.pdf
// Xilinx Series7 Configuation documented at:
//     ug470_7Series_Config.pdf

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ftdi_reference.h"

/*
 * Generic FTDI initialization
 */
struct ftdi_context *init_ftdi(uint8_t *data, int size)
{
    struct ftdi_device_list *devlist, *curdev;
    char serial[64], manuf[64], desc[128];
    int i;

    /*
     * Locate USB interface for JTAG
     */
    struct ftdi_context *ftdi = ftdi_new();
    ftdi_init(ftdi);
    ftdi_usb_find_all(ftdi, &devlist, 0x0, 0x0);
    curdev = devlist;
    ftdi_usb_get_strings(ftdi, curdev->dev, manuf, sizeof(manuf),
        desc, sizeof(desc), serial, sizeof(serial));
    printf("[%s] %s:%s:%s\n", __FUNCTION__, manuf, desc, serial);
    ftdi_usb_open_dev(ftdi, curdev->dev);
    ftdi_usb_reset(ftdi);
    ftdi_list_free(&devlist);

    /*
     * Identify JTAG interface version
     */
    int eeprom_val;
    uint8_t fbuf[256]; // since chiptype is 0x56, eerom size is 256
    ftdi_read_eeprom(ftdi);
    ftdi_eeprom_decode(ftdi, 0);
    ftdi_get_eeprom_value(ftdi, CHIP_TYPE, &eeprom_val);
    printf("[%s:%d] CHIP_TYPE %x\n", __FUNCTION__, __LINE__, eeprom_val);
    ftdi_get_eeprom_buf(ftdi, fbuf, sizeof(fbuf));

    /*
     * Generic initialization of libftdi
     */
    ftdi_set_baudrate(ftdi, 9600);
    ftdi_set_latency_timer(ftdi, 255);
    ftdi_set_bitmode(ftdi, 0, 0);
    ftdi_set_bitmode(ftdi, 0, 2);
    ftdi_usb_purge_buffers(ftdi);
    ftdi_usb_purge_rx_buffer(ftdi);
    ftdi_usb_purge_tx_buffer(ftdi);

    /*
     * Generic command synchronization with ftdi chip
     */
    static uint8_t errorcode_fa[] = { 0xfa };
    for (i = 0; i < 4; i++) {
        static uint8_t illegal_command[] = { 0xaa, SEND_IMMEDIATE };
        ftdi_write_data(ftdi, illegal_command, sizeof(illegal_command));
        ftdi_read_data(ftdi, errorcode_fa, sizeof(errorcode_fa));
        static uint8_t readdata1z[] = { 0xaa };
        ftdi_read_data(ftdi, readdata1z, sizeof(readdata1z));
    }
    static uint8_t command_ab[] = { 0xab, SEND_IMMEDIATE };
    ftdi_write_data(ftdi, command_ab, sizeof(command_ab));
    ftdi_read_data(ftdi, errorcode_fa, sizeof(errorcode_fa));
    static uint8_t readdata2z[] = { 0xab };
    ftdi_read_data(ftdi, readdata2z, sizeof(readdata2z));
    if (size)
        ftdi_write_data(ftdi, data, size);
    return ftdi;
}

#define BUFFER_MAX_LEN      1000000
#define FILE_READSIZE          6464
#define MAX_SINGLE_USB_DATA    4045
#define TOKENPASTE4(A, B, C, D) (A ## B ## C ## D)
#define M(A)               ((A) & 0xff)
#define INT16(A)           M(A), M((A) >> 8)
#define INT32(A)           INT16(A), INT16((A) >> 16)
#define BSWAP(A) ((((A) & 1) << 7) | (((A) & 2) << 5) | (((A) & 4) << 3) | (((A) & 8) << 1) \
         | (((A) & 0x10) >> 1) | (((A) & 0x20) >> 3) | (((A) & 0x40) >> 5) | (((A) & 0x80) >> 7))
#define MS(A)              BSWAP(M(A))
#define SWAP32(A)          MS((A) >> 24), MS((A) >> 16), MS((A) >> 8), MS(A)
#define SWAP32B(A)         MS(A), MS((A) >> 8), MS((A) >> 16), MS((A) >> 24)

/*
 * Xilinx constants
 */
// IDCODE from bsd file
#define IDCODE_XC7K325T    TOKENPASTE4(  \
      /*XXXX         / version */        \
      0b0011011,     /* family */        \
        001010001,   /* array size */    \
        00001001001, /* manufacturer */  \
        1)           /* required by 1149.1 */
#define IDCODE_VERSION     0x40000000

#define IDCODE_VALUE INT32(IDCODE_VERSION | IDCODE_XC7K325T)

#define SET_CLOCK_DIVISOR    0x86, INT16(1)   // 15MHz (since disable clk divide by 5)

#define IRREG_USER2          0x003
#define IRREG_CFG_OUT        0x004
#define IRREG_CFG_IN         0x005
#define IRREG_USERCODE       0x008
#define IRREG_JPROGRAM       0x00b
#define IRREG_JSTART         0x00c
#define IRREG_ISC_NOOP       0x014
#define IRREG_BYPASS         0x13f

#define SMAP_DUMMY           0xffffffff
#define SMAP_SYNC            0xaa995566

// Type 1 Packet
#define SMAP_TYPE1(OPCODE,REG,COUNT) \
    (0x20000000 | ((OPCODE) << 27) | ((REG) << 13) | (COUNT))
#define SMAP_OP_NOP         0
#define SMAP_OP_READ        1
#define SMAP_OP_WRITE       2
#define SMAP_REG_CMD     0x04  // CMD register, Table 5-22
#define     SMAP_CMD_DESYNC 0x0000000d  // end of configuration
#define SMAP_REG_STAT    0x07  // STAT register, Table 5-25
#define SMAP_REG_BOOTSTS 0x16  // BOOTSTS register, Table 5-35

// Type 2 Packet
#define SMAP_TYPE2(LEN) (0x40000000 | (LEN))

/*
 * FTDI constants
 */
#define MREAD   (MPSSE_LSB|MPSSE_READ_NEG)
#define MWRITE  (MPSSE_LSB|MPSSE_WRITE_NEG)
#define DREAD   (MPSSE_DO_READ  | MREAD)
#define DWRITE  (MPSSE_DO_WRITE | MWRITE)

#define TMSW  (MPSSE_WRITE_TMS      |MWRITE|MPSSE_BITMODE)//4b
#define TMSRW (MPSSE_WRITE_TMS|DREAD|MWRITE|MPSSE_BITMODE)//6f

#define DATAWBIT  (DWRITE|MPSSE_BITMODE)       //1b
#define DATARBIT  (DREAD |MPSSE_BITMODE)       //2e
#define DATARWBIT (DREAD |DWRITE|MPSSE_BITMODE)//3f
#define DATAW(A)          DWRITE, INT16((A)-1) //19
#define DATAR(A)           DREAD, INT16((A)-1) //2c
#define DATARW(A) (DREAD|DWRITE), INT16((A)-1) //3d
#define PULSE_CLOCK        0x8f

#define IDLE_TO_SHIFT_IR   TMSW, 0x03, 0x03  /* Idle -> Shift-IR */
#define IDLE_TO_SHIFT_DR   TMSW, 0x02, 0x01  /* Idle -> Shift-DR */
#define EXIT1_TO_IDLE      TMSW, 0x01, 0x01  /* Exit1/Exit2 -> Idle */
#define IDLE_TO_RESET      TMSW, 0x02, 0x07  /* Idle -> Reset */
#define RESET_TO_IDLE      TMSW, 0x00, 0x00  /* Reset -> Idle */
#define IN_RESET_STATE     TMSW, 0x00, 0x7f  /* Marker for Reset */
#define SHIFT_TO_EXIT1(A) \
     TMSW, 0x00, ((A) | 0x01)             /* Shift-IR -> Exit1-IR */
#define SHIFT_TO_EXIT1_RW(A) \
     TMSRW, 0x00, ((A) | 0x01)            /* Shift-IR -> Exit1-IR */
#define SHIFT_TO_UPDATE_TO_IDLE(A) \
     TMSW, 0x02, ((A) | 0x03)    /* Shift-DR -> Update-DR -> Idle */
#define SHIFT_TO_UPDATE_TO_IDLE_RW(A) \
     TMSRW, 0x02, ((A) | 0x03)   /* Shift-DR -> Update-DR -> Idle */
#define FORCE_RETURN_TO_RESET TMSW, 0x04, 0x1f /* go back to TMS reset state */

#define JTAG_IRREG(A)                             \
     IDLE_TO_SHIFT_IR,                            \
     DATAWBIT, 0x04, M(A),                        \
     SHIFT_TO_EXIT1(((A) & 0x100)>>1),            \
     EXIT1_TO_IDLE

#define JTAG_IRREG_RW(A)                          \
     IDLE_TO_SHIFT_IR,                            \
     DATARWBIT, 0x04, M(A),                       \
     SHIFT_TO_EXIT1_RW(((A) & 0x100)>>1),         \
     SEND_IMMEDIATE

#define EXTENDED_COMMAND(A)                       \
     IDLE_TO_SHIFT_IR,                            \
     DATAWBIT, 0x04, M(0xc0 | (A)),               \
     SHIFT_TO_UPDATE_TO_IDLE(((A) & 0x100)>>1)

#define EXTENDED_COMMAND_RW(A)                    \
     IDLE_TO_SHIFT_IR,                            \
     DATARWBIT, 0x04, M(0xc0 | (A)),              \
     SHIFT_TO_UPDATE_TO_IDLE_RW(((A) & 0x100)>>1)

#define PATTERN1 \
         INT32(0xff), INT32(0xff), INT32(0xff), INT32(0xff), INT32(0xff), \
         INT32(0xff), INT32(0xff), INT32(0xff), INT32(0xff), INT32(0xff), \
         INT32(0xff), INT32(0xff), INT32(0xff), INT32(0xff), INT32(0xff)

#define PATTERN2 \
         INT32(0xffffffff), INT32(0xffffffff), INT32(0xffffffff), INT32(0xffffffff), \
         INT32(0xffffffff), INT32(0xffffffff), INT32(0xffffffff), INT32(0xffffffff), \
         INT32(0xffffffff), INT32(0xffffffff), INT32(0xffffffff), INT32(0xffffffff), \
         INT32(0xffffffff), INT32(0xffffffff), INT32(0xffffffff), INT32(0xffffffff), \
         INT32(0xffffffff), INT32(0xffffffff), INT32(0xffffffff), INT32(0xffffffff), \
         INT32(0xffffffff), INT32(0xffffffff), INT32(0xffffffff), INT32(0xffffffff), \
         INT32(0xffffffff), INT32(0xffffffff), INT32(0xffffffff), INT32(0xffffffff), \
         INT32(0xffffffff), INT32(0xffffffff), INT32(0xffffffff)
#define DITEM(...) ((uint8_t[]){sizeof((uint8_t[]){ __VA_ARGS__ }), __VA_ARGS__})

#define WRITE_READ(FTDI, A, B) \
    writetc = ftdi_write_data_submit(FTDI, (A)+1, (A)[0]); \
    check_ftdi_read_data_submit(FTDI, B);

static void memdump(uint8_t *p, int len, char *title)
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


static uint8_t *pulse_gpio(int delay)
{
#define GPIO_DONE            0x10
#define GPIO_01              0x01
#define SET_LSB_DIRECTION(A) 0x80, 0xe0, (0xea | (A))

    static uint8_t prebuffer[BUFFER_MAX_LEN];
    static uint8_t pulsepre[] =
      DITEM(SET_LSB_DIRECTION(GPIO_DONE | GPIO_01),
            SET_LSB_DIRECTION(GPIO_DONE));
    static uint8_t pulse65k[] = DITEM(PULSE_CLOCK, INT16(65536 - 1));
    static uint8_t pulsepost[] =
      DITEM(SET_LSB_DIRECTION(GPIO_DONE | GPIO_01),
            SET_LSB_DIRECTION(GPIO_01));
    uint8_t *ptr = prebuffer+1;
    memcpy(ptr, pulsepre+1, pulsepre[0]);
    ptr += pulsepre[0];
    while(delay > 65536) {
        memcpy(ptr, pulse65k+1, pulse65k[0]);
        ptr += pulse65k[0];
        delay -= 65536;
    }
    *ptr++ = PULSE_CLOCK;
    *ptr++ = M(delay-1);
    *ptr++ = M((delay-1)>>8);
    memcpy(ptr, pulsepost+1, pulsepost[0]);
    ptr += pulsepost[0];
    prebuffer[0] = ptr - (prebuffer + 1);
    return prebuffer;
}

static struct ftdi_transfer_control* writetc;
static uint8_t bitswap[256];
static uint8_t last_read_data[10000];

static void check_ftdi_read_data_submit(struct ftdi_context *ftdi, uint8_t *buf)
{
    struct ftdi_transfer_control* tc = ftdi_read_data_submit(ftdi, last_read_data, buf[0]);
    if (writetc)
        ftdi_transfer_data_done(writetc);
    writetc = NULL;
    ftdi_transfer_data_done(tc);
    if (memcmp(buf+1, last_read_data, buf[0])) {
        printf("[%s:%d]\n", __FUNCTION__, __LINE__);
        memdump(buf+1, buf[0], "EXPECT");
        memdump(last_read_data, buf[0], "ACTUAL");
    }
}

static void send_data_frame(struct ftdi_context *ftdi, uint8_t read, uint8_t *header,
    uint8_t *tail, uint8_t *ptrin, int size, int limit_len)
{
    int i;
    static uint8_t packetbuffer[BUFFER_MAX_LEN];
    uint8_t *readptr = packetbuffer;

    memcpy(readptr, header+1, header[0]);
    readptr += header[0];
    while (size > 0) {
        int rlen = size-1;
        if (rlen > limit_len)
            rlen = limit_len;
        if (rlen < limit_len)
            rlen--;                   // last byte is actually loaded with DATAW command
        *readptr++ = DWRITE | read;
        *readptr++ = rlen;
        *readptr++ = rlen >> 8;
        for (i = 0; i <= rlen; i++)
            *readptr++ = *ptrin++;
        if (rlen < limit_len) {
            uint8_t ch = *ptrin++;
            *readptr++ = DATAWBIT | read;
            *readptr++ = 0x06;
            *readptr++ = ch;        // 7 bits of data here
            memcpy(readptr, tail+1, tail[0]);
            *(readptr+2) |= 0x80 & ch; // insert 1 bit of data here
            readptr += tail[0];
        }
        writetc = ftdi_write_data_submit(ftdi, packetbuffer, readptr - packetbuffer);
        ftdi_transfer_data_done(writetc);
        writetc = NULL;
        size -= limit_len+1;
        readptr = packetbuffer;
    }
}

#define COMMAND_ENDING  /* Enters in Shift-DR */            \
     DATAR(3),                                              \
     DATARBIT, 0x06,                                        \
     SHIFT_TO_UPDATE_TO_IDLE_RW(0),                         \
     SEND_IMMEDIATE

static void test_pattern(struct ftdi_context *ftdi)
{
#define DATA_ITEM(...) \
     EXTENDED_COMMAND(IRREG_BYPASS), \
     EXTENDED_COMMAND(IRREG_USER2),  \
     IDLE_TO_SHIFT_DR,               \
     __VA_ARGS__                     \
     COMMAND_ENDING

    static uint8_t readdata_five_zeros[] = DITEM( INT32(0), 0x00 );
    int i;

    WRITE_READ(ftdi, DITEM( DATA_ITEM( ) ), readdata_five_zeros);
    WRITE_READ(ftdi, DITEM( DATA_ITEM( DATAW(1), 0x69,       /* in Shift-DR */
                    DATAWBIT, 0x01, 0x00, )), /* in Shift-DR */ \
         readdata_five_zeros);
    for (i = 0; i < 2; i++) {
        WRITE_READ(ftdi, DITEM( DATA_ITEM(
                    DATAWBIT, 0x04, 0x0c, /* in Shift-DR */
                    SHIFT_TO_UPDATE_TO_IDLE(0),
                    IDLE_TO_SHIFT_DR,
                    DATAW(1), 0x69,       /* in Shift-DR */
                    DATAWBIT, 0x01, 0x00, )), /* in Shift-DR */
          readdata_five_zeros);
    }
}

#define IDTEST_PATTERN1                         \
     TMSW, 0x04, 0x7f, /* Reset????? */         \
     TMSW, 0x03, 0x02  /* Reset -> Shift-DR */

static uint8_t patdata[] =  {PATTERN1, INT32(0xff)};
static uint8_t pat3[] = DITEM( SHIFT_TO_UPDATE_TO_IDLE_RW(0), SEND_IMMEDIATE);
static uint8_t readdata3z[] = DITEM( IDCODE_VALUE, PATTERN1, 0x00 );
static void test_idcode(struct ftdi_context *ftdi)
{
    int j;
    send_data_frame(ftdi, DREAD, DITEM( IDLE_TO_RESET, IDTEST_PATTERN1),
        pat3, patdata, sizeof(patdata), 9999);
    check_ftdi_read_data_submit(ftdi, readdata3z);
    for (j = 0; j < 3; j++)
        test_pattern(ftdi);
}

static void check_idcode(struct ftdi_context *ftdi, int instance)
{
    int j = 3, k = 2;

    send_data_frame(ftdi, DREAD, DITEM( TMSW, 0x00, 0x01, IDTEST_PATTERN1),
        pat3, patdata, sizeof(patdata), 9999);
    check_ftdi_read_data_submit(ftdi, readdata3z);
    if(instance) {
        while (j-- > 0)
            test_pattern(ftdi);
        k = 3;
    }
    while (k-- > 0)
        test_idcode(ftdi);
}

static void read_status(struct ftdi_context *ftdi, int instance)
{
uint8_t *data;
int i;

#define READ_STAT_REG1(...)          \
     IDLE_TO_RESET,                  \
     __VA_ARGS__                     \
     RESET_TO_IDLE,                  \
     EXTENDED_COMMAND(IRREG_CFG_IN), \
     IDLE_TO_SHIFT_DR

static uint8_t request_data[] = {
     SWAP32(SMAP_DUMMY), SWAP32(SMAP_SYNC), SWAP32(SMAP_TYPE2(0)),
     SWAP32(SMAP_TYPE1(SMAP_OP_READ, SMAP_REG_STAT, 1)), SWAP32(0)};

    if (!instance)
        data = DITEM( READ_STAT_REG1() );
    else
        data = DITEM(READ_STAT_REG1(IN_RESET_STATE, TMSW, 0x00, 0x01, ));  /* ... -> Reset */
    send_data_frame(ftdi, 0, data,
        DITEM(SHIFT_TO_UPDATE_TO_IDLE(0),
            EXTENDED_COMMAND(IRREG_CFG_OUT),
            IDLE_TO_SHIFT_DR,
            COMMAND_ENDING),
        request_data, sizeof(request_data), 9999);
    check_ftdi_read_data_submit(ftdi, DITEM( 2, SWAP32B(0xf0fe7910) ));
    union {
        uint32_t i;
        uint8_t  c[4];
    } status;
    for (i = 0; i < 4; i++)
        status.c[i] = bitswap[last_read_data[i+1]];
    printf("STATUS %08x done %x release_done %x eos %x startup_state %x\n", status.i,
        status.i & 0x4000, status.i & 0x2000, status.i & 0x10, (status.i >> 18) & 7);
    static uint8_t itor[] = { IDLE_TO_RESET };
    ftdi_transfer_data_done(ftdi_write_data_submit(ftdi, itor, sizeof(itor)));
}

static uint8_t *catlist(uint8_t *arg[])
{
    static uint8_t prebuffer[BUFFER_MAX_LEN];
    uint8_t *ptr = prebuffer + 1;
    while (*arg) {
        memcpy(ptr, *arg+1, (*arg)[0]);
        ptr += (*arg)[0];
        arg++;
    }
    prebuffer[0] = ptr - (prebuffer + 1);
    return prebuffer;
}
static void send_smap(struct ftdi_context *ftdi, uint8_t *prefix, uint32_t data, uint8_t *rdata)
{
    uint8_t temp[] = {4, SWAP32(data)};
    static uint8_t request_data[] = {INT32(4)};
    uint8_t *alist[] = {prefix,
        DITEM(
            JTAG_IRREG(IRREG_CFG_IN),
            IDLE_TO_SHIFT_DR,
            DATAW(4), SWAP32(SMAP_DUMMY),
            DATAW(4), SWAP32(SMAP_SYNC),
            DATAW(4), SWAP32(SMAP_TYPE1(SMAP_OP_NOP, 0,0)),
            DATAW(4)),
        temp,
        DITEM(
            DATAW(4), SWAP32(SMAP_TYPE1(SMAP_OP_NOP, 0,0)),
            DATAW(4), SWAP32(SMAP_TYPE1(SMAP_OP_NOP, 0,0)),
            DATAW(4), SWAP32(SMAP_TYPE1(SMAP_OP_WRITE, SMAP_REG_CMD, 1)),
            DATAW(4), SWAP32(SMAP_CMD_DESYNC),
            DATAW(4), SWAP32(SMAP_TYPE1(SMAP_OP_NOP, 0,0))), 0};
    uint8_t *p = catlist(alist);

    send_data_frame(ftdi, 0, p, DITEM(
         SHIFT_TO_EXIT1(0),
         EXIT1_TO_IDLE,
         JTAG_IRREG(IRREG_CFG_OUT),
         IDLE_TO_SHIFT_DR,
         DATARW(3), 0x00, 0x00, 0x00,
         DATARWBIT, 0x06, 0x00,
         SHIFT_TO_EXIT1_RW(0),
         SEND_IMMEDIATE ), request_data, sizeof(request_data), 9999);
    check_ftdi_read_data_submit(ftdi, rdata);
}

int main(int argc, char **argv)
{
    struct ftdi_context *ctxitem0z;
    int i;

    if (argc != 2) {
        printf("tester <filename>\n");
        exit(1);
    }
    for (i = 0; i < sizeof(bitswap); i++)
        bitswap[i] = BSWAP(i);

    /*
     * Initialize FTDI chip and GPIO pins
     */
    static uint8_t initialize_sequence[] = {
         0x85, // Disconnect TDI/DO from loopback
         0x8a, // Disable clk divide by 5
         SET_CLOCK_DIVISOR,
         0x80, 0xe8, 0xeb,
         0x82, 0x20, 0x30,
         0x82, 0x30, 0x00,
         0x82, 0x00, 0x00,
         FORCE_RETURN_TO_RESET
    };
    ctxitem0z = init_ftdi(initialize_sequence, sizeof(initialize_sequence));

    check_idcode(ctxitem0z, 0);
    static uint8_t item8z[] = { IDLE_TO_RESET, IN_RESET_STATE};
    writetc = ftdi_write_data_submit(ctxitem0z, item8z, sizeof(item8z));
    ftdi_transfer_data_done(writetc);
    static uint8_t command_set_divisor[] = { SET_CLOCK_DIVISOR };
    ftdi_write_data(ctxitem0z, command_set_divisor, sizeof(command_set_divisor));

    /*
     * Step 5: Check Device ID
     */
    static uint8_t iddata[] = { PATTERN2, INT32(0xffffffff) };
    send_data_frame(ctxitem0z, DREAD,
        DITEM(TMSW, 0x00, 0x01,  /* ... -> Reset */
             IN_RESET_STATE,
             TMSW, 0x00, 0x01,  /* ... -> Reset */
             IN_RESET_STATE,
             RESET_TO_IDLE,
             IDLE_TO_SHIFT_DR),
        DITEM(TMSRW, 0x01, 0x01, /* Shift-DR -> Pause-DR */
             SEND_IMMEDIATE),
        iddata, sizeof(iddata), 9999);
    check_ftdi_read_data_submit(ctxitem0z, DITEM( IDCODE_VALUE, PATTERN2, 0xff ));

    WRITE_READ(ctxitem0z, DITEM(
         FORCE_RETURN_TO_RESET,
         IN_RESET_STATE,
         RESET_TO_IDLE,
         EXTENDED_COMMAND(IRREG_USERCODE),
         IDLE_TO_SHIFT_DR,
         COMMAND_ENDING), DITEM( 0xff, INT32(0xffffffff) ));
    for (i = 0; i < 3; i++) {
        WRITE_READ(ctxitem0z, DITEM( EXTENDED_COMMAND_RW(IRREG_BYPASS), SEND_IMMEDIATE ),
            DITEM( INT16(0xf5af) ));
    }
    read_status(ctxitem0z, 0);
    check_idcode(ctxitem0z, 1);

    /*
     * Step 2: Initialization
     */
    uint8_t *alist[] = {DITEM(
             IDLE_TO_RESET, IN_RESET_STATE, RESET_TO_IDLE,
             JTAG_IRREG(IRREG_JPROGRAM),
             JTAG_IRREG(IRREG_ISC_NOOP)),
         pulse_gpio(15000000/80) /* 12.5 msec */,
         DITEM( JTAG_IRREG_RW(IRREG_ISC_NOOP) ), 0};
    uint8_t *p = catlist(alist);
    writetc = ftdi_write_data_submit(ctxitem0z, p+1, p[0]);
    check_ftdi_read_data_submit(ctxitem0z, DITEM( INT16(0x4488) ));

    /*
     * Step 6: Load Configuration Data Frames
     */
    WRITE_READ(ctxitem0z, DITEM( EXIT1_TO_IDLE, JTAG_IRREG_RW(IRREG_CFG_IN) ),
        DITEM( INT16(0x458a) ));

    printf("Starting to send file '%s'\n", argv[1]);
    int inputfd = open(argv[1], O_RDONLY);
    uint8_t *headerp =
         DITEM( EXIT1_TO_IDLE, IDLE_TO_SHIFT_DR, DATAW(4), INT32(0) );
    int limit_len = MAX_SINGLE_USB_DATA - headerp[0];
    static uint8_t TMS_STATE_1_0[] = DITEM(TMSW, 0x01, 0x01);
    uint8_t *tailp = TMS_STATE_1_0;      /* Shift-DR -> Pause-DR */
    int last = 0;
    while (!last) {
        static uint8_t filebuffer[FILE_READSIZE];
        int size = read(inputfd, filebuffer, FILE_READSIZE);
        last = (size < FILE_READSIZE);
        if (last)
            tailp = DITEM(TMSW, 0x00, 0x01,/* Shift-DR -> Exit1-DR */
                          EXIT1_TO_IDLE);
        for (i = 0; i < size; i++)
            filebuffer[i] = bitswap[filebuffer[i]];
        send_data_frame(ctxitem0z, 0, headerp, tailp, filebuffer, size, limit_len);
        limit_len = MAX_SINGLE_USB_DATA;
        headerp = TMS_STATE_1_0;         /* Pause-DR -> Shift-DR */
    }
    printf("[%s:%d] done sending file\n", __FUNCTION__, __LINE__);

    /*
     * Step 8: Startup
     */
    send_smap(ctxitem0z, pulse_gpio(15000000/800),  // 1.25 msec
         SMAP_TYPE1(SMAP_OP_READ, SMAP_REG_BOOTSTS, 1), DITEM( INT32(0), 0x80 ));

    WRITE_READ(ctxitem0z, DITEM(
         EXIT1_TO_IDLE,
         JTAG_IRREG(IRREG_BYPASS),
         JTAG_IRREG(IRREG_JSTART),
         TMSW, 0x00, 0x00,  /* Hang out in Idle for a while */
         TMSW, 0x06, 0x00, TMSW, 0x06, 0x00, TMSW, 0x06, 0x00,
         TMSW, 0x06, 0x00, TMSW, 0x06, 0x00, TMSW, 0x06, 0x00,
         TMSW, 0x06, 0x00, TMSW, 0x06, 0x00, TMSW, 0x06, 0x00,
         TMSW, 0x06, 0x00, TMSW, 0x06, 0x00, TMSW, 0x06, 0x00,
         TMSW, 0x06, 0x00, TMSW, 0x06, 0x00,
         TMSW, 0x01, 0x00,
         JTAG_IRREG_RW(IRREG_BYPASS)), DITEM( INT16(0xd6ac) ));

    send_smap(ctxitem0z, DITEM( EXIT1_TO_IDLE ),
         SMAP_TYPE1(SMAP_OP_READ, SMAP_REG_STAT, 1), DITEM( 0x02, SWAP32B(0xfcfe7910) ));

    static uint8_t item20z[] = { EXIT1_TO_IDLE, JTAG_IRREG(IRREG_BYPASS) };
    writetc = ftdi_write_data_submit(ctxitem0z, item20z, sizeof(item20z));
    ftdi_transfer_data_done(writetc);

    WRITE_READ(ctxitem0z, DITEM(
         IDLE_TO_RESET, IN_RESET_STATE, RESET_TO_IDLE,
         EXTENDED_COMMAND_RW(IRREG_BYPASS),
         SEND_IMMEDIATE), DITEM( INT16(0xf5a9) ));
    test_idcode(ctxitem0z);
    read_status(ctxitem0z, 1);
    ftdi_deinit(ctxitem0z);
    return 0;
}
