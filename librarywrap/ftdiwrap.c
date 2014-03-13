
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

#include "ftdi_reference.h"

#define ACCUM_LIMIT 0xfffffff //12000

static void memdump(const unsigned char *p, int len, const char *title);
static void dump_context(struct ftdi_context *p);
static void final_dump(void);
static char *writedata(int submit, const unsigned char *buf, int size);
static char *translate_context(struct ftdi_context *p);
static char *readdata(const unsigned char *buf, int size);
static char *translate_buffer(void *p, int len);

static struct ftdi_transfer_control *read_data_submit_control;
static struct ftdi_context *read_data_context;
static unsigned char *read_data_buffer;
static int read_data_len;
static struct ftdi_transfer_control *write_data_submit_control;
static int write_data_len;
static long accum;
static struct ftdi_context *master_ftdi;
static int datafile_fd = -1;

#include "ftdiwrap.h"

#define MAX_ITEMS 1000

static struct {
    unsigned char *p;
    int len;
} strarr[MAX_ITEMS];
static int strarr_index = 0;
static struct {
    unsigned char *p;
    int len;
} readarr[MAX_ITEMS];
static int readarr_index = 0;
static struct ftdi_context *ctxarr[MAX_ITEMS];
static int ctxarr_index = 0;
static struct {
    void *p;
    int len;
} bufarr[MAX_ITEMS];
static int bufarr_index = 0;
static int logall = 1;

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

static char *translate_buffer(void *p, int len)
{
static char tempstr[200];
int i = 0;
while (i < bufarr_index) {
    if (p == bufarr[i].p && bufarr[i].len == len)
        break;
    i++;
}
if (i == bufarr_index) {
    bufarr[bufarr_index].p = p;
    bufarr[bufarr_index++].len = len;
}
sprintf(tempstr, "bufitem%dz, sizeof(bufitem%dz)", i, i);
return strdup(tempstr);
}

static void memdump(const unsigned char *p, int len, const char *title)
{
int i;

    i = 0;
    while (len > 0) {
        if (!(i & 0xf)) {
            if (i > 0)
                fprintf(logfile, "\n");
            fprintf(logfile, "%s ",title);
        }
        fprintf(logfile, "0x%02x, ", *p++);
        i++;
        len--;
    }
    fprintf(logfile, "\n");
}
static int started = 0;
static void formatwrite(int submit, const unsigned char *p, int len, const char *title)
{
   static unsigned char bitswap[256];
   static int once = 1;
   static const char *header = "    ";
   static char header_data[200];
   if (logall)
       header = "WRITE";
   strcpy(header_data, header);
   strcat(header_data, "   ");
   
    while (len > 0) {
        const unsigned char *pstart = p;
        int plen = 1;
        unsigned char ch = *p;
        switch(ch) {
        case 0x85: case 0x87: case 0x8a: case 0xaa: case 0xab:
            break;
        case 0x2e:
            plen = 2;
            break;
        case 0x19: case 0x1b: case 0x2c: case 0x3d: case 0x3f: case 0x4b:
        case 0x6f: case 0x80: case 0x82: case 0x86: case 0x8f:
            plen = 3;
            break;
        default:
            memdump(p-1, len, title);
            return;
        }
        if (!submit || accum < ACCUM_LIMIT)
            memdump(pstart, plen, header);
        if (started && p[0] == 0x1b && p[1] == 6)
            write(datafile_fd, &bitswap[p[2]], 1);
        p += plen;
        len -= plen;
        if (ch == 0x19 || ch == 0x3d) {
            unsigned tlen = (pstart[2] << 8 | pstart[1]) + 1;
if (tlen > 1500) {
    started = 1;
}
else
    started = 0;    // shutdown before final writes
            if (!started)
                memdump(p, tlen, header_data);
            //if (submit && tlen > 4) 
            else {
                int i;
                for (i = 0; once && i < sizeof(bitswap); i++)
                    bitswap[i] = ((i &    1) << 7) | ((i &    2) << 5)
                       | ((i &    4) << 3) | ((i &    8) << 1)
                       | ((i & 0x10) >> 1) | ((i & 0x20) >> 3)
                       | ((i & 0x40) >> 5) | ((i & 0x80) >> 7);
                unsigned char *pbuf = (unsigned char *)malloc(tlen);
                for (i = 0; i < tlen; i++)
                    pbuf[i] = bitswap[p[i]];
                write(datafile_fd, pbuf, tlen);
                free(pbuf);
                once = 0;
            }
            p += tlen;
            len -= tlen;
        }
    }
    if (len != 0)
        printf("[%s] ending length %d\n", __FUNCTION__, len);
}
#define FILE_BYTE_SIZE  500

static char *writedata(int submit, const unsigned char *buf, int size)
{
    static char tempbuf[200];
        int ind = -1;
        int i = 0;
        if ((submit && started) || logall)
            formatwrite(submit, buf, size, "WRITE");
        else {
            while (i < strarr_index) {
                if (strarr[i].len == size && !memcmp(buf, strarr[i].p, size)) {
                    ind = i;
                    break;
                }
                i++;
            }
            if (i == strarr_index) {
                strarr[i].p = (unsigned char *)malloc(size);
                strarr[i].len = size;
                memcpy(strarr[i].p, buf, size);
                ind = strarr_index++;
            }
            sprintf(tempbuf, "item%dz, sizeof(item%dz)", ind, ind);
            return tempbuf;
        }
    accum += size;
    sprintf(tempbuf, "%p, %d", buf, size);
if (size < 1000)
    started = 0;    // shutdown before final writes
    return tempbuf;
}
static char *readdata(const unsigned char *buf, int size)
{
    static char tempbuf[200];
    int ind = -1;
    int i = 0;
    if (logall)
        memdump(buf, size, "READ");
    else {
        while (i < readarr_index) {
            if (readarr[i].len == size && !memcmp(buf, readarr[i].p, size)) {
                ind = i;
                break;
            }
            i++;
        }
        if (i == readarr_index) {
            readarr[i].p = (unsigned char *)malloc(size);
            readarr[i].len = size;
            memcpy(readarr[i].p, buf, size);
            ind = readarr_index++;
        }
    }
    sprintf(tempbuf, "readdata%dz, sizeof(readdata%dz)", ind, ind);
    return tempbuf;
}

static void final_dump(void)
{
    int i = 0;

    accum = 0;
    if (datafile_fd >= 0)
        close(datafile_fd);
    datafile_fd = -1;
    while (i < ctxarr_index)
        fprintf(logfile, "static struct ftdi_context *ctxitem%dz;\n", i++);
    i = 0;
    while (i < bufarr_index) {
        fprintf(logfile, "static unsigned char bufitem%dz[%d];\n", i, bufarr[i].len);
        i++;
    }
    i = 0;
    while (i < strarr_index) {
        fprintf(logfile, "static unsigned char item%dz[] = {\n", i);
        formatwrite(0, strarr[i].p, strarr[i].len, "STRARR");
        fprintf(logfile, "};\n");
        i++;
    }
    i = 0;
    while (i < readarr_index) {
        fprintf(logfile, "static unsigned char readdata%dz[] = {\n", i);
        memdump(readarr[i].p, readarr[i].len, "    ");
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
