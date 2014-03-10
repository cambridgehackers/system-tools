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
struct ftdi_context *init_ftdi(void)
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
     * Identify JTAG interface chip version
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
    return ftdi;
}

#define BUFFER_MAX_LEN      1000000
#define FILE_READSIZE          6464
#define MAX_SINGLE_USB_DATA    4045
#define DITEM(...) ((uint8_t[]){sizeof((uint8_t[]){ __VA_ARGS__ }), __VA_ARGS__})
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
#define IDCODE_XC7K325T  (  \
     /*               XXXX  /  version */        \
          (0b0011011 << 21) /* family */        \
     |  (0b001010001 << 12) /* array size */    \
     |(0b00001001001 <<  1) /* manufacturer */  \
     |                   1) /* required by 1149.1 */
#define IDCODE_VERSION     0x40000000

#define IDCODE_VALUE INT32(IDCODE_VERSION | IDCODE_XC7K325T)

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

#define IDLE_TO_SHIFT_IR   TMSW, 0x03, 0x03  /* Idle -> Shift-IR */
#define IDLE_TO_SHIFT_DR   TMSW, 0x02, 0x01  /* Idle -> Shift-DR */
#define EXIT1_TO_IDLE      TMSW, 0x01, 0x01  /* Exit1/Exit2 -> Idle */
#define IDLE_TO_RESET      TMSW, 0x02, 0x07  /* Idle -> Reset */
#define RESET_TO_IDLE      TMSW, 0x00, 0x00  /* Reset -> Idle */
#define IN_RESET_STATE     TMSW, 0x00, 0x7f  /* Marker for Reset */
#define SHIFT_TO_EXIT1(READA, A) \
     TMSW | (READA), 0x00, ((A) | 0x01)             /* Shift-IR -> Exit1-IR */
#define SHIFT_TO_UPDATE_TO_IDLE(READA, A) \
     TMSW | (READA), 0x02, ((A) | 0x03)    /* Shift-DR -> Update-DR -> Idle */
#define FORCE_RETURN_TO_RESET TMSW, 0x04, 0x1f /* go back to TMS reset state */
#define RESET_TO_SHIFT_DR     TMSW, 0x03, 0x02  /* Reset -> Shift-DR */
#define RESET_TO_RESET        TMSW, 0x00, 0x01
#define TMSW_DELAY                                             \
         TMSW, 0x00, 0x00,  /* Hang out in Idle for a while */ \
         TMSW, 0x06, 0x00, TMSW, 0x06, 0x00, TMSW, 0x06, 0x00, \
         TMSW, 0x06, 0x00, TMSW, 0x06, 0x00, TMSW, 0x06, 0x00, \
         TMSW, 0x06, 0x00, TMSW, 0x06, 0x00, TMSW, 0x06, 0x00, \
         TMSW, 0x06, 0x00, TMSW, 0x06, 0x00, TMSW, 0x06, 0x00, \
         TMSW, 0x06, 0x00, TMSW, 0x06, 0x00,                   \
         TMSW, 0x01, 0x00
#define PAUSE_TO_SHIFT       TMSW, 0x01, 0x01 /* Pause-DR -> Shift-DR */
#define SHIFT_TO_PAUSE       TMSW, 0x01, 0x01 /* Shift-DR -> Pause-DR */
#define TMS_RESET_WEIRD      TMSW, 0x04, 0x7f /* Reset????? */
#define EXTEND_EXTRA 0xc0

#define JTAG_IRREG(READA, A)                             \
     IDLE_TO_SHIFT_IR,                            \
     DATAWBIT | (READA), 0x04, M(A),                        \
     SHIFT_TO_EXIT1((READA), ((A) & 0x100)>>1)

#define EXTENDED_COMMAND(READA, A)                       \
     IDLE_TO_SHIFT_IR,                            \
     DATAWBIT | (READA), 0x04, M(A),                 \
     SHIFT_TO_UPDATE_TO_IDLE((READA), ((A) & 0x100)>>1)

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

static uint8_t *pulse_gpio(int delay)
{
#define GPIO_DONE            0x10
#define GPIO_01              0x01
#define SET_LSB_DIRECTION(A) SET_BITS_LOW, 0xe0, (0xea | (A))

    static uint8_t prebuffer[BUFFER_MAX_LEN];
    static uint8_t pulsepre[] =
      DITEM(SET_LSB_DIRECTION(GPIO_DONE | GPIO_01),
            SET_LSB_DIRECTION(GPIO_DONE));
    static uint8_t pulse65k[] = DITEM(CLK_BYTES, INT16(65536 - 1));
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
    *ptr++ = CLK_BYTES;
    *ptr++ = M(delay-1);
    *ptr++ = M((delay-1)>>8);
    memcpy(ptr, pulsepost+1, pulsepost[0]);
    ptr += pulsepost[0];
    prebuffer[0] = ptr - (prebuffer + 1);
    return prebuffer;
}

static void data_submit(struct ftdi_context *ftdi, uint8_t *buf, int size)
{
    struct ftdi_transfer_control* writetc = ftdi_write_data_submit(ftdi, buf, size);
    ftdi_transfer_data_done(writetc);
}

static uint8_t *check_ftdi_read_data_submit(struct ftdi_context *ftdi, uint8_t *buf)
{
    static uint8_t last_read_data[10000];
    struct ftdi_transfer_control* tc = ftdi_read_data_submit(ftdi, last_read_data, buf[0]);
    ftdi_transfer_data_done(tc);

    if (memcmp(buf+1, last_read_data, buf[0])) {
        printf("[%s:%d]\n", __FUNCTION__, __LINE__);
        memdump(buf+1, buf[0], "EXPECT");
        memdump(last_read_data, buf[0], "ACTUAL");
    }
    return last_read_data;
}

static void WRITE_READ(struct ftdi_context *ftdi, uint8_t *req, uint8_t *resp)
{
    data_submit(ftdi, req+1, req[0]);
    if (resp)
        check_ftdi_read_data_submit(ftdi, resp);
}

static uint8_t *send_data_frame(struct ftdi_context *ftdi, uint8_t read_param, uint8_t *headerl[],
    uint8_t *tail, uint8_t *ptrin, int size, int limit_len, uint8_t *checkdata)
{
    int i;
    static uint8_t packetbuffer[BUFFER_MAX_LEN];
    uint8_t *readptr = packetbuffer;

    uint8_t *header = catlist(headerl);
    memcpy(readptr, header+1, header[0]);
    readptr += header[0];
    while (size > 0) {
        int rlen = size-1;
        if (rlen > limit_len)
            rlen = limit_len;
        if (rlen < limit_len)
            rlen--;                   // last byte is actually loaded with DATAW command
        *readptr++ = DWRITE | read_param;
        *readptr++ = rlen;
        *readptr++ = rlen >> 8;
        for (i = 0; i <= rlen; i++)
            *readptr++ = *ptrin++;
        if (rlen < limit_len) {
            uint8_t ch = *ptrin++;
            *readptr++ = DATAWBIT | read_param;
            *readptr++ = 0x06;
            *readptr++ = ch;        // 7 bits of data here
            memcpy(readptr, tail+1, tail[0]);
            *readptr |= read_param; // this is a TMS instruction to shift state
            *(readptr+2) |= 0x80 & ch; // insert 1 bit of data here
            readptr += tail[0];
        }
        data_submit(ftdi, packetbuffer, readptr - packetbuffer);
        size -= limit_len+1;
        readptr = packetbuffer;
    }
    if(checkdata)
        return check_ftdi_read_data_submit(ftdi, checkdata);
    return NULL;
}

static uint8_t bitswap[256];
static void send_data_file(struct ftdi_context *ftdi, const char *filename)
{
    int size, i;
    uint8_t *tailp = DITEM(SHIFT_TO_PAUSE);
    uint8_t *headerp =
         DITEM( EXIT1_TO_IDLE, IDLE_TO_SHIFT_DR, DATAW(4), INT32(0) );
    int limit_len = MAX_SINGLE_USB_DATA - headerp[0];

    printf("Starting to send file '%s'\n", filename);
    int inputfd = open(filename, O_RDONLY);
    do {
        static uint8_t filebuffer[FILE_READSIZE];
        size = read(inputfd, filebuffer, FILE_READSIZE);
        if (size < FILE_READSIZE)
            tailp = DITEM(SHIFT_TO_EXIT1(0, 0), EXIT1_TO_IDLE);
        for (i = 0; i < size; i++)
            filebuffer[i] = bitswap[filebuffer[i]];
        send_data_frame(ftdi, 0, (uint8_t *[]){headerp, NULL}, tailp, filebuffer, size, limit_len, NULL);
        headerp = DITEM(PAUSE_TO_SHIFT);
        limit_len = MAX_SINGLE_USB_DATA;
    } while(size == FILE_READSIZE);
    printf("[%s:%d] done sending file\n", __FUNCTION__, __LINE__);
}

#define COMMAND_ENDING  /* Enters in Shift-DR */            \
     DATAR(3),                                              \
     DATARBIT, 0x06,                                        \
     SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0),                     \
     SEND_IMMEDIATE


static void check_idcode(struct ftdi_context *ftdi, uint8_t *statep)
{
    static uint8_t patdata[] =  {INT32(0xff), PATTERN1};

    send_data_frame(ftdi, DREAD,
        (uint8_t *[]){statep,
                      DITEM(TMS_RESET_WEIRD, RESET_TO_SHIFT_DR),
                      NULL},
        DITEM( SHIFT_TO_UPDATE_TO_IDLE(0, 0), SEND_IMMEDIATE),
        patdata, sizeof(patdata), 9999, DITEM( IDCODE_VALUE, PATTERN1, 0x00));
}

static void bypass_test(struct ftdi_context *ftdi, uint8_t *statep)
{
    uint8_t *added_item[] = {
        DITEM( DATAW(1), 0x69, DATAWBIT, 0x01, 0x00, ),
        DITEM( DATAWBIT, 0x04, 0x0c, SHIFT_TO_UPDATE_TO_IDLE(0, 0), IDLE_TO_SHIFT_DR)};
    static uint8_t readdata_five_zeros[] = DITEM( INT32(0), 0x00 );
    int i, j = 3;
    check_idcode(ftdi, statep);
    while (j-- > 0) {
        uint8_t *alist[5] = {
            DITEM( EXTENDED_COMMAND(0, EXTEND_EXTRA | IRREG_BYPASS),
                   EXTENDED_COMMAND(0, EXTEND_EXTRA | IRREG_USER2),
                   IDLE_TO_SHIFT_DR),
            DITEM(COMMAND_ENDING),
            NULL, NULL, NULL};
        for (i = 0; i < 4; i++) {
           WRITE_READ(ftdi, catlist(alist), readdata_five_zeros);
           if (i <= 1) {
               alist[3] = alist[2];
               alist[2] = alist[1];
               alist[1] = added_item[i];
           }
        }
    }
}

static void read_status(struct ftdi_context *ftdi, uint8_t *stat2, uint8_t *stat3)
{
    int i;
    union {
        uint32_t i;
        uint8_t  c[4];
    } status;
    static uint8_t request_data[] = {
         SWAP32(SMAP_DUMMY), SWAP32(SMAP_SYNC), SWAP32(SMAP_TYPE2(0)),
         SWAP32(SMAP_TYPE1(SMAP_OP_READ, SMAP_REG_STAT, 1)), SWAP32(0)};

    uint8_t *lastp = send_data_frame(ftdi, 0,
        (uint8_t *[]){DITEM(IDLE_TO_RESET), stat2, stat3, NULL},
        DITEM(SHIFT_TO_UPDATE_TO_IDLE(0, 0),
            EXTENDED_COMMAND(0, EXTEND_EXTRA | IRREG_CFG_OUT),
            IDLE_TO_SHIFT_DR,
            COMMAND_ENDING),
        request_data, sizeof(request_data), 9999, DITEM( 2, SWAP32B(0xf0fe7910) ));
    for (i = 0; i < 4; i++)
        status.c[i] = bitswap[lastp[i+1]];
    printf("STATUS %08x done %x release_done %x eos %x startup_state %x\n", status.i,
        status.i & 0x4000, status.i & 0x2000, status.i & 0x10, (status.i >> 18) & 7);
    WRITE_READ(ftdi, DITEM( IDLE_TO_RESET ), NULL);
}

static void send_smap(struct ftdi_context *ftdi, uint8_t *prefix, uint32_t data, uint8_t *rdata)
{
    static uint8_t request_data[] = {INT32(4)};

    send_data_frame(ftdi, 0,
        (uint8_t *[]){prefix,
                      DITEM(JTAG_IRREG(0, IRREG_CFG_IN), EXIT1_TO_IDLE,
                        IDLE_TO_SHIFT_DR,
                        DATAW(4), SWAP32(SMAP_DUMMY),
                        DATAW(4), SWAP32(SMAP_SYNC),
                        DATAW(4), SWAP32(SMAP_TYPE1(SMAP_OP_NOP, 0,0)),
                        DATAW(4)),
                      (uint8_t []){4, SWAP32(data)},
                      DITEM(DATAW(4), SWAP32(SMAP_TYPE1(SMAP_OP_NOP, 0,0)),
                        DATAW(4), SWAP32(SMAP_TYPE1(SMAP_OP_NOP, 0,0)),
                        DATAW(4), SWAP32(SMAP_TYPE1(SMAP_OP_WRITE, SMAP_REG_CMD, 1)),
                        DATAW(4), SWAP32(SMAP_CMD_DESYNC),
                        DATAW(4), SWAP32(SMAP_TYPE1(SMAP_OP_NOP, 0,0))), NULL},
        DITEM(SHIFT_TO_EXIT1(0, 0), EXIT1_TO_IDLE,
              JTAG_IRREG(0, IRREG_CFG_OUT), EXIT1_TO_IDLE,
              IDLE_TO_SHIFT_DR,
              DATARW(3), 0x00, 0x00, 0x00,
              DATARWBIT, 0x06, 0x00,
              SHIFT_TO_EXIT1(DREAD, 0),
              SEND_IMMEDIATE ),
        request_data, sizeof(request_data), 9999, rdata);
}

static uint8_t *cfg_in_command = DITEM(RESET_TO_IDLE, EXTENDED_COMMAND(0, EXTEND_EXTRA | IRREG_CFG_IN), IDLE_TO_SHIFT_DR);
int main(int argc, char **argv)
{
    struct ftdi_context *ftdi;
    int i;

    if (argc != 2) {
        printf("tester <filename>\n");
        exit(1);
    }

#define CLOCK_FREQUENCY      15000000
#define SET_CLOCK_DIVISOR    0x86, INT16(30000000/CLOCK_FREQUENCY - 1)
    /*
     * Initialize FTDI chip and GPIO pins
     */
    ftdi = init_ftdi();   /* generic initialization */
    static uint8_t initialize_sequence[] = {
         LOOPBACK_END, // Disconnect TDI/DO from loopback
         DIS_DIV_5, // Disable clk divide by 5
         SET_CLOCK_DIVISOR,
         SET_BITS_LOW, 0xe8, 0xeb,
         SET_BITS_HIGH, 0x20, 0x30,
         SET_BITS_HIGH, 0x30, 0x00,
         SET_BITS_HIGH, 0x00, 0x00,
         FORCE_RETURN_TO_RESET
    };
    ftdi_write_data(ftdi, initialize_sequence, sizeof(initialize_sequence));
    for (i = 0; i < sizeof(bitswap); i++)
        bitswap[i] = BSWAP(i);

    /*
     * Step 5: Check Device ID
     */
    check_idcode(ftdi, DITEM( RESET_TO_RESET));
    bypass_test(ftdi, DITEM( IDLE_TO_RESET));
    bypass_test(ftdi, DITEM( IDLE_TO_RESET));

    WRITE_READ(ftdi,
       DITEM(IDLE_TO_RESET, IN_RESET_STATE),
       NULL);
    static uint8_t command_set_divisor[] = { SET_CLOCK_DIVISOR };
    ftdi_write_data(ftdi, command_set_divisor, sizeof(command_set_divisor));

    static uint8_t iddata[] = {INT32(0xffffffff),  PATTERN2};
    send_data_frame(ftdi, DREAD,
        (uint8_t *[]){DITEM(RESET_TO_RESET, IN_RESET_STATE,
             RESET_TO_RESET, IN_RESET_STATE,
             RESET_TO_IDLE, IDLE_TO_SHIFT_DR), NULL},
        DITEM(PAUSE_TO_SHIFT, SEND_IMMEDIATE),
        iddata, sizeof(iddata), 9999, DITEM( IDCODE_VALUE, PATTERN2, 0xff ));

    WRITE_READ(ftdi,
        DITEM(FORCE_RETURN_TO_RESET, IN_RESET_STATE, RESET_TO_IDLE,
            EXTENDED_COMMAND(0, EXTEND_EXTRA | IRREG_USERCODE),
            IDLE_TO_SHIFT_DR,
            COMMAND_ENDING),
        DITEM( 0xff, INT32(0xffffffff) ));
    for (i = 0; i < 3; i++)
        WRITE_READ(ftdi,
            DITEM( EXTENDED_COMMAND(DREAD, EXTEND_EXTRA | IRREG_BYPASS), SEND_IMMEDIATE ),
            DITEM( INT16(0x1188) ));
            //DITEM( INT16(0xf5af) )); // when already programmed
    read_status(ftdi, cfg_in_command, NULL);
    bypass_test(ftdi, DITEM( RESET_TO_RESET));
    bypass_test(ftdi, DITEM( IDLE_TO_RESET));
    bypass_test(ftdi, DITEM( IDLE_TO_RESET));
    bypass_test(ftdi, DITEM( IDLE_TO_RESET));

    /*
     * Step 2: Initialization
     */
    WRITE_READ(ftdi,
        catlist((uint8_t *[]){
            DITEM(IDLE_TO_RESET, IN_RESET_STATE, RESET_TO_IDLE,
               JTAG_IRREG(0, IRREG_JPROGRAM), EXIT1_TO_IDLE,
               JTAG_IRREG(0, IRREG_ISC_NOOP), EXIT1_TO_IDLE),
            pulse_gpio(CLOCK_FREQUENCY/80) /* 12.5 msec */,
            DITEM( JTAG_IRREG(DREAD, IRREG_ISC_NOOP), SEND_IMMEDIATE),
            NULL}),
       DITEM( INT16(0x4488) ));

    /*
     * Step 6: Load Configuration Data Frames
     */
    WRITE_READ(ftdi,
        DITEM( EXIT1_TO_IDLE, JTAG_IRREG(DREAD, IRREG_CFG_IN), SEND_IMMEDIATE),
        DITEM( INT16(0x458a) ));
    send_data_file(ftdi, argv[1]);

    /*
     * Step 8: Startup
     */
    send_smap(ftdi,
        pulse_gpio(CLOCK_FREQUENCY/800),  // 1.25 msec
        SMAP_TYPE1(SMAP_OP_READ, SMAP_REG_BOOTSTS, 1),
        DITEM( 0, SWAP32B(0x1000000) ));

    WRITE_READ(ftdi,
        DITEM(
            EXIT1_TO_IDLE,
            JTAG_IRREG(0, IRREG_BYPASS), EXIT1_TO_IDLE,
            JTAG_IRREG(0, IRREG_JSTART), EXIT1_TO_IDLE,
            TMSW_DELAY,
            JTAG_IRREG(DREAD, IRREG_BYPASS), SEND_IMMEDIATE),
        DITEM( INT16(0xd6ac) ));

    send_smap(ftdi,
        DITEM( EXIT1_TO_IDLE ),
        SMAP_TYPE1(SMAP_OP_READ, SMAP_REG_STAT, 1),
        DITEM( 0x02, SWAP32B(0xfcfe7910) ));

    WRITE_READ(ftdi, DITEM(EXIT1_TO_IDLE, JTAG_IRREG(0, IRREG_BYPASS), EXIT1_TO_IDLE), NULL);
    WRITE_READ(ftdi,
        DITEM(IDLE_TO_RESET, IN_RESET_STATE, RESET_TO_IDLE,
              EXTENDED_COMMAND(DREAD, EXTEND_EXTRA | IRREG_BYPASS),
              SEND_IMMEDIATE),
        DITEM( INT16(0xf5a9) ));
    bypass_test(ftdi, DITEM( IDLE_TO_RESET));
    read_status(ftdi, DITEM(IN_RESET_STATE, RESET_TO_RESET), cfg_in_command);
    ftdi_deinit(ftdi);
    return 0;
}
