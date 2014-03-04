
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

#include "ftdi_reference.h"

#define ACCUM_LIMIT 12000

static void memdump(const unsigned char *p, int len, const char *title);
static void dump_context(struct ftdi_context *p);
static void final_dump(void);
static char *writedata(int submit, const unsigned char *buf, int size);
static char *translate_context(struct ftdi_context *p);
static char *readdata(const unsigned char *buf, int size);
static char *translate_buffer(void *p, int len);
static void end_datafile();

static struct ftdi_transfer_control *read_data_submit_control;
static struct ftdi_context *read_data_context;
static unsigned char *read_data_buffer;
static int read_data_len;
static struct ftdi_transfer_control *write_data_submit_control;
static int write_data_len;
static long accum;
static struct ftdi_context *master_ftdi;
static int datafile_fd = -1;
static int datafile_index;

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
static void formatwrite(const unsigned char *p, int len, const char *title)
{
#if 0
    if (accum >= ACCUM_LIMIT) {
        accum = 0;
        fprintf(logfile, "\n");
    }
#endif
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
            memdump(p, tlen > 64 ? 64 : tlen, "         ");
            p += tlen;
            len -= tlen;
        }
    }
    if (len != 0)
        printf("[%s] ending length %d\n", __FUNCTION__, len);
}
#define FILE_BYTE_SIZE  500
static unsigned char data1[] = { /* first packet */
    0x4b, 0x1, 0x1,
    0x4b, 0x2, 0x1,
    0x19, 0x3, 0x0,
        0x0, 0x0, 0x0, 0x0,
    0x19, 0xc0, 0x0f};
static unsigned char data2[] = { /* short start */
    0x19, 0x7d, 0x09};
static unsigned char data3[] = { /* short end */
    0x1b, 0x6, 0,
    0x4b, 1, 1};
static unsigned char data4[] = { /* long start */
    0x4b, 1, 1,
    0x19, 0xcd, 0x0f};

static char *writedata(int submit, const unsigned char *buf, int size)
{
static unsigned char bitswap[256];
static int once = 1;
    static char tempbuf[200];
    if (submit && datafile_fd < 0 && size >= FILE_BYTE_SIZE) {
        sprintf(tempbuf, "/tmp/xx.datafile%d", datafile_index++);
        fprintf(logfile, "[%s] opening file '%s'\n", __FUNCTION__, tempbuf);
        datafile_fd = creat(tempbuf, 0666);
    }
    if (submit && datafile_fd >= 0) {
        int i, tsize = size;
        for (i = 0; once && i < sizeof(bitswap); i++)
            bitswap[i] = ((i &    1) << 7) | ((i &    2) << 5)
                       | ((i &    4) << 3) | ((i &    8) << 1)
                       | ((i & 0x10) >> 1) | ((i & 0x20) >> 3)
                       | ((i & 0x40) >> 5) | ((i & 0x80) >> 7);
        unsigned char *p = (unsigned char *)malloc(size);
        unsigned char *pdata = p;
        for (i = 0; i < size; i++)
            p[i] = bitswap[buf[i]];
        if (once) {
            if (memcmp(buf, data1, sizeof(data1)))
                memdump(buf, sizeof(data1), "DATA1");
            pdata += sizeof(data1);
            tsize -= sizeof(data1);
        }
        else if (tsize > 4000) {
            if (memcmp(buf, data4, sizeof(data4)))
                memdump(buf, sizeof(data4), "DATA4");
            pdata += 6;
            tsize -= 6;
        }
        else {
            if (memcmp(buf, data2, sizeof(data2)))
                memdump(buf, sizeof(data2), "DATA2");
            if (memcmp(&buf[size - sizeof(data3)], data3, sizeof(data3)))
                memdump(&buf[size - sizeof(data3)], sizeof(data3), "DATA3");
            pdata += 3;
            tsize -= 9;
        }
        write(datafile_fd, pdata, tsize);
        free(p);
        once = 0;
    }
    if (accum < ACCUM_LIMIT) {
        int ind = -1;
        int i = 0;
        if (size > FILE_BYTE_SIZE)
            formatwrite(buf, size, "WRITE");
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
    }
    accum += size;
    sprintf(tempbuf, "%p, %d", buf, size);
    return tempbuf;
}
static char *readdata(const unsigned char *buf, int size)
{
    static char tempbuf[200];
    int ind = -1;
    int i = 0;
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
    sprintf(tempbuf, "readdata%dz, sizeof(readdata%dz)", ind, ind);
    return tempbuf;
}

static void end_datafile()
{
    if (accum >= ACCUM_LIMIT)
        fprintf(logfile, "\n");
    accum = 0;
    if (datafile_fd >= 0)
        close(datafile_fd);
    datafile_fd = -1;
}
static void final_dump(void)
{
int i = 0;
end_datafile();
while (i < ctxarr_index)
    fprintf(logfile, "static struct ftdi_context *ctxitem%dz;\n", i++);
i = 0;
while (i < bufarr_index) {
    fprintf(logfile, "static unsigned char bufitem%dz[%d];\n", i, bufarr[i].len);
    i++;
}
i = 0;
while (i < strarr_index) {
    fprintf(logfile, "static unsigned char item%dz = {\n", i);
    formatwrite(strarr[i].p, strarr[i].len, "STRARR");
    fprintf(logfile, "};\n");
    i++;
}
i = 0;
while (i < readarr_index) {
    fprintf(logfile, "static unsigned char readdata%dz = {\n", i);
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
