
#include "ftdi_reference.h"

static void memdump(const unsigned char *p, int len, char *title);
static void formatwrite(const unsigned char *p, int len, char *title);
static void dump_context(struct ftdi_context *p);
static struct ftdi_transfer_control *read_data_submit_control;
static unsigned char *read_data_buffer;
static int read_data_len;
static struct ftdi_transfer_control *write_data_submit_control;
static int write_data_len;
static long accum;

#include "ftdiwrap.h"

static void memdump(const unsigned char *p, int len, char *title)
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
static void formatwrite(const unsigned char *p, int len, char *title)
{
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

static void dump_context(struct ftdi_context *p)
{
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
