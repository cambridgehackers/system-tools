
#include <malloc.h>
#include <string.h>

#include "ftdi_reference.h"

#define ACCUM_LIMIT 12000

static void memdump(const unsigned char *p, int len, const char *title);
static void dump_context(struct ftdi_context *p);
static void final_dump(void);
static char *writedata(const char *fn, const unsigned char *buf, int size);
static char *translate_context(struct ftdi_context *p);

static struct ftdi_transfer_control *read_data_submit_control;
static unsigned char *read_data_buffer;
static int read_data_len;
static struct ftdi_transfer_control *write_data_submit_control;
static int write_data_len;
static long accum;
static struct ftdi_context *master_ftdi;

#include "ftdiwrap.h"

#define MAX_ITEMS 1000

static struct {
    unsigned char *p;
    int len;
} strarr[MAX_ITEMS];
static int strarr_index = 0;
struct ftdi_context *ctxarr[MAX_ITEMS];
static int ctxarr_index = 0;

static char *translate_context(struct ftdi_context *p)
{
static char tempstr[200];
int i = 0;
while (i < ctxarr_index) {
    if (p == ctxarr[i])
        break;
    i++;
}
if (i == ctxarr_index)
    ctxarr[ctxarr_index++] = p;
sprintf(tempstr, "ctxitem%dz", i);
return tempstr;
}

static int lookup(const unsigned char *buf, int len)
{
int i = 0;
if (len > 500)
    return -1;
while (i < strarr_index) {
    if (strarr[i].len == len && !memcmp(buf, strarr[i].p, len))
        return i;
    i++;
}
if (i < MAX_ITEMS) {
    strarr[i].p = (unsigned char *)malloc(len);
    strarr[i].len = len;
    memcpy(strarr[i].p, buf, len);
    return strarr_index++;
}
return -1;
}

static void memdump(const unsigned char *p, int len, const char *title)
{
int i;

    i = 0;
    while (len > 0) {
        if (!(i & 0xf)) {
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
static void formatwrite(const unsigned char *p, int len, const char *title)
{
    if (accum >= ACCUM_LIMIT) {
        accum = 0;
        fprintf(logfile, "\n");
    }
    while (len > 0) {
        const unsigned char *pstart = p;
        int plen = 1;
        unsigned char ch = *p;
        switch(ch) {
        case 0x85: case 0x87: case 0x8a: case 0xaa: case 0xab:
            break;
        case 0x2e: case 0x3d:
            plen = 2;
            break;
        case 0x19: case 0x1b: case 0x2c: case 0x3f: case 0x4b:
        case 0x6f: case 0x80: case 0x82: case 0x86: case 0x8f:
            plen = 3;
            break;
        case 0x00: case 0xff:
            plen = 4;
            break;
        default:
            memdump(p-1, len, title);
            return;
        }
        memdump(pstart, plen, "    ");
        p += plen;
        len -= plen;
        if (ch == 0x19) {
            unsigned tlen = (pstart[2] << 8 | pstart[1]) + 1;
            memdump(p, tlen > 64 ? 64 : tlen, "        WDATA");
            p += tlen;
            len -= tlen;
        }
    }
    if (len != 0)
        printf("[%s] ending length %d\n", __FUNCTION__, len);
}
static char *writedata(const char *fn, const unsigned char *buf, int size)
{
    static char tempbuf[200];
    if (size < 1000)
        accum = 0;
    if (accum < ACCUM_LIMIT) {
        int ind = lookup(buf, size);
        if (ind == -1)
            formatwrite(buf, size, fn);
        else
            fprintf(logfile, "%s item %d\n", fn, ind);
    }
    accum += size;
    sprintf(tempbuf, "%p, %d", buf, size);
    return tempbuf;
}
static void final_dump(void)
{
int i = 0;
while (i < ctxarr_index)
    fprintf(logfile, "static struct ftdi_context *ctxitem%dz;\n", i++);
i = 0;
while (i < strarr_index) {
    fprintf(logfile, "static unsigned char item%dz = {\n", i);
    formatwrite(strarr[i].p, strarr[i].len, "STRARR");
    fprintf(logfile, "};\n");
    i++;
}
}

static void dump_context(struct ftdi_context *p)
{
return;
    fprintf(logfile, "context: %p\n", p);
    fprintf(logfile, "    .usb_ctx: %p,\n", p->usb_ctx);
    fprintf(logfile, "    .usb_dev: %p,\n", p->usb_dev);
    fprintf(logfile, "    .usb_read_timeout: %d,\n", p->usb_read_timeout);
    fprintf(logfile, "    .usb_write_timeout: %d,\n", p->usb_write_timeout);
    //fprintf(logfile, "    .ftdi_chip_type: %d,\n", p->ftdi_chip_type);
    fprintf(logfile, "    .baudrate: %d,\n", p->baudrate);
    fprintf(logfile, "    .bitbang_enabled: %d,\n", p->bitbang_enabled);
    fprintf(logfile, "    .readbuffer: %p,\n", p->readbuffer);
    fprintf(logfile, "    .readbuffer_offset: %d,\n", p->readbuffer_offset);
    fprintf(logfile, "    .readbuffer_remaining: %d,\n", p->readbuffer_remaining);
    fprintf(logfile, "    .readbuffer_chunksize: %d,\n", p->readbuffer_chunksize);
    fprintf(logfile, "    .writebuffer_chunksize: %d,\n", p->writebuffer_chunksize);
    fprintf(logfile, "    .max_packet_size: %d,\n", p->max_packet_size);
    fprintf(logfile, "    .interface: %d,\n", p->interface);
    fprintf(logfile, "    .index: %d,\n", p->index);
    fprintf(logfile, "    .in_ep: %d,\n", p->in_ep);
    fprintf(logfile, "    .out_ep: %d,\n", p->out_ep);
    fprintf(logfile, "    .bitbang_mode: %d,\n", p->bitbang_mode);
    fprintf(logfile, "    .eeprom: %p,\n", p->eeprom);
    fprintf(logfile, "    .error_str: %p,\n", p->error_str);
}
