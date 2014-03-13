/***************************************************************************
    Derived from ftdi.c
    copyright            : (C) 2003-2014 by Intra2net AG and the libftdi developers
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation;             *
 *                                                                         *
 ***************************************************************************/
#include <libusb.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#define WRITE_TIMEOUT   5000
#define READ_TIMEOUT    5000
#define ENDPOINT_IN     0x02
#define ENDPOINT_OUT    0x81
#define USB_INDEX       0
#define WRITE_CHUNKSIZE 4096
#define READ_CHUNKSIZE  4096
enum ftdi_chip_type {
    TYPE_AM=0, TYPE_BM=1, TYPE_2232C=2, TYPE_R=3, TYPE_2232H=4, TYPE_4232H=5,
    TYPE_232H=6, TYPE_230X=7, };
enum ftdi_mpsse_mode { BITMODE_RESET  = 0x00,    BITMODE_MPSSE  = 0x02,    };
enum ftdi_interface { INTERFACE_ANY = 0, INTERFACE_A   = 1, };
#define MPSSE_WRITE_NEG 0x01   /* Write TDI/DO on negative TCK/SK edge*/
#define MPSSE_BITMODE   0x02   /* Write bits, not bytes */
#define MPSSE_READ_NEG  0x04   /* Sample TDO/DI on negative TCK/SK edge */
#define MPSSE_LSB       0x08   /* LSB first */
#define MPSSE_DO_WRITE  0x10   /* Write TDI/DO */
#define MPSSE_DO_READ   0x20   /* Read TDO/DI */
#define MPSSE_WRITE_TMS 0x40   /* Write TMS/CS */
#define SET_BITS_LOW   0x80
#define SET_BITS_HIGH  0x82
#define GET_BITS_LOW   0x81
#define GET_BITS_HIGH  0x83
#define LOOPBACK_END   0x85
#define TCK_DIVISOR    0x86
#define DIS_DIV_5       0x8a
#define CLK_BYTES       0x8f
#define DIV_VALUE(rate) (rate > 6000000)?0:((6000000/rate -1) > 0xffff)? 0xffff: (6000000/rate -1)
#define SEND_IMMEDIATE 0x87
#define SIO_RESET          0 /* Reset the port */
#define SIO_SET_BAUD_RATE  3 /* Set baud rate */
#define SIO_SET_DATA       4 /* Set the data characteristics of the port */
#define FTDI_DEVICE_OUT_REQTYPE (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT)
#define FTDI_DEVICE_IN_REQTYPE (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN)
#define SIO_RESET_REQUEST             SIO_RESET
#define SIO_SET_BAUDRATE_REQUEST      SIO_SET_BAUD_RATE
#define SIO_SET_DATA_REQUEST          SIO_SET_DATA
#define SIO_SET_LATENCY_TIMER_REQUEST 0x09
#define SIO_SET_BITMODE_REQUEST       0x0B
#define SIO_RESET_SIO 0
#define SIO_RESET_PURGE_RX 1
#define SIO_RESET_PURGE_TX 2
struct ftdi_context {
    struct libusb_context *usb_ctx;
    struct libusb_device_handle *usb_dev;
    enum ftdi_chip_type type;
    unsigned char readbuffer[READ_CHUNKSIZE];
};
struct ftdi_device_list {
    struct ftdi_device_list *next;
    struct libusb_device *dev;
};
#define ftdi_error_return(code, str) do {  \
        fprintf(stderr, str);          \
        return code;                       \
   } while(0);
#define ftdi_error_return_free_device_list(code, str, devs) do {    \
        libusb_free_device_list(devs,1);   \
        fprintf(stderr, str);          \
        return code;                       \
   } while(0);
static int ftdi_usb_find_all(struct ftdi_context *ftdi, struct ftdi_device_list **devlist, int vendor, int product)
{
    struct ftdi_device_list **curdev;
    libusb_device *dev;
    libusb_device **devs;
    struct libusb_device_descriptor desc;

    int count = 0;
    int i = 0;
    if (libusb_get_device_list(ftdi->usb_ctx, &devs) < 0)
        ftdi_error_return(-5, "libusb_get_device_list() failed");
    curdev = devlist;
    *curdev = NULL;
    while ((dev = devs[i++]) ) {
        if (libusb_get_device_descriptor(dev, &desc) < 0)
            ftdi_error_return_free_device_list(-6, "libusb_get_device_descriptor() failed", devs);
        if ( desc.idVendor == 0x403 && (desc.idProduct == 0x6001 || desc.idProduct == 0x6010
                     || desc.idProduct == 0x6011 || desc.idProduct == 0x6014)) {
            *curdev = (struct ftdi_device_list*)malloc(sizeof(struct ftdi_device_list));
            if (!*curdev)
                ftdi_error_return_free_device_list(-3, "out of memory", devs);
            (*curdev)->next = NULL;
            (*curdev)->dev = dev;
            libusb_ref_device(dev);
            curdev = &(*curdev)->next;
            count++;
        }
    }
    libusb_free_device_list(devs,1);
    return count;
}
static void ftdi_list_free(struct ftdi_device_list **devlist)
{
    struct ftdi_device_list *curdev, *next;
    for (curdev = *devlist; curdev;) {
        next = curdev->next;
        libusb_unref_device(curdev->dev);
        free(curdev);
        curdev = next;
    }
    *devlist = NULL;
}
static void ftdi_usb_close_internal (struct ftdi_context *ftdi)
{
    if (ftdi && ftdi->usb_dev) {
        libusb_close (ftdi->usb_dev);
        ftdi->usb_dev = NULL;
    }
}
static void ftdi_deinit(struct ftdi_context *ftdi)
{
    if (ftdi == NULL)
        return;
    ftdi_usb_close_internal (ftdi);
    if (ftdi->usb_ctx) {
        libusb_exit(ftdi->usb_ctx);
        ftdi->usb_ctx = NULL;
    }
}
static int ftdi_usb_get_strings(struct ftdi_context * ftdi, struct libusb_device * dev,
     char * manufacturer, int mnf_len, char * description, int desc_len, char * serial, int serial_len)
{
    struct libusb_device_descriptor desc;
    if (libusb_open(dev, &ftdi->usb_dev) < 0)
        ftdi_error_return(-4, "libusb_open() failed");
    if (libusb_get_device_descriptor(dev, &desc) < 0)
        ftdi_error_return(-11, "libusb_get_device_descriptor() failed");
    if (libusb_get_string_descriptor_ascii(ftdi->usb_dev, desc.iManufacturer, (unsigned char *)manufacturer, mnf_len) < 0
     || libusb_get_string_descriptor_ascii(ftdi->usb_dev, desc.iProduct, (unsigned char *)description, desc_len) < 0
     || libusb_get_string_descriptor_ascii(ftdi->usb_dev, desc.iSerialNumber, (unsigned char *)serial, serial_len) < 0) {
        ftdi_usb_close_internal (ftdi);
        ftdi_error_return(-9, "libusb_get_string_descriptor_ascii() failed");
    }
    ftdi_usb_close_internal (ftdi);
    return 0;
}
static int ftdi_usb_purge_buffers(struct ftdi_context *ftdi)
{
    if (libusb_control_transfer(ftdi->usb_dev, FTDI_DEVICE_OUT_REQTYPE, SIO_RESET_REQUEST, SIO_RESET_PURGE_RX, USB_INDEX, NULL, 0, WRITE_TIMEOUT) < 0)
        ftdi_error_return(-1, "FTDI purge of RX buffer failed");
    if (libusb_control_transfer(ftdi->usb_dev, FTDI_DEVICE_OUT_REQTYPE, SIO_RESET_REQUEST, SIO_RESET_PURGE_TX, USB_INDEX, NULL, 0, WRITE_TIMEOUT) < 0)
        ftdi_error_return(-1, "FTDI purge of TX buffer failed");
    return 0;
}
static int ftdi_set_baudrate(struct ftdi_context *ftdi, int baudrate)
{
    static const char frac_code[8] = {0, 3, 2, 4, 1, 5, 6, 7};
    int best_divisor = 12000000*8 / baudrate;
    unsigned long encoded_divisor = (best_divisor >> 3) | (frac_code[best_divisor & 0x7] << 14);
    if (libusb_control_transfer(ftdi->usb_dev, FTDI_DEVICE_OUT_REQTYPE, SIO_SET_BAUDRATE_REQUEST, (encoded_divisor | 0x20000) & 0xFFFF,
        ((encoded_divisor >> 8) & 0xFF00) | USB_INDEX, NULL, 0, WRITE_TIMEOUT) < 0)
        ftdi_error_return (-2, "Setting new baudrate failed");
    return 0;
}
static int ftdi_write_data(struct ftdi_context *ftdi, const unsigned char *buf, int size)
{
    int actual_length;
    if (libusb_bulk_transfer(ftdi->usb_dev, ENDPOINT_IN, (unsigned char *)buf, size, &actual_length, WRITE_TIMEOUT) < 0)
        ftdi_error_return(-1, "usb bulk write failed");
    return actual_length;
}
static struct ftdi_transfer_control *ftdi_write_data_submit(struct ftdi_context *ftdi, unsigned char *buf, int size)
{
    ftdi_write_data(ftdi, buf, size);
    return NULL;
}
static void memdump(unsigned char *p, int len, char *title);
static int ftdi_read_data(struct ftdi_context *ftdi, unsigned char *buf, int size)
{
    int offset = 0, ret;
    int actual_length = 1;
    do {
        ret = libusb_bulk_transfer (ftdi->usb_dev, ENDPOINT_OUT, ftdi->readbuffer, READ_CHUNKSIZE, &actual_length, READ_TIMEOUT);
        if (ret < 0)
            ftdi_error_return(ret, "usb bulk read failed");
        actual_length -= 2;
    } while (actual_length == 0);
    memcpy (buf, ftdi->readbuffer+2, actual_length);
    if (actual_length != size) {
        printf("[%s:%d] bozo actual_length %d size %d\n", __FUNCTION__, __LINE__, actual_length, size);
        exit(-1);
        }
    return offset;
}
static int ftdi_transfer_data_done(struct ftdi_transfer_control *tc)
{
    return 0;
}
static struct ftdi_transfer_control *ftdi_read_data_submit(struct ftdi_context *ftdi, unsigned char *buf, int size)
{
    ftdi_read_data(ftdi, buf, size);
    return NULL;
}
static int ftdi_set_bitmode(struct ftdi_context *ftdi, unsigned char bitmask, unsigned char mode)
{
    if (libusb_control_transfer(ftdi->usb_dev, FTDI_DEVICE_OUT_REQTYPE, SIO_SET_BITMODE_REQUEST, bitmask | (mode << 8), USB_INDEX, NULL, 0, WRITE_TIMEOUT) < 0)
        ftdi_error_return(-1, "unable to configure mode");
    return 0;
}
static int ftdi_set_latency_timer(struct ftdi_context *ftdi, unsigned char latency)
{
    if (libusb_control_transfer(ftdi->usb_dev, FTDI_DEVICE_OUT_REQTYPE, SIO_SET_LATENCY_TIMER_REQUEST, latency, USB_INDEX, NULL, 0, WRITE_TIMEOUT) < 0)
        ftdi_error_return(-2, "unable to set latency timer");
    return 0;
}
static int ftdi_usb_open_dev(struct ftdi_context *ftdi, libusb_device *dev)
{
    struct libusb_device_descriptor desc;
    struct libusb_config_descriptor *config0;
    int cfg, cfg0;

    if (libusb_open(dev, &ftdi->usb_dev) < 0
     || libusb_get_device_descriptor(dev, &desc) < 0
     || libusb_get_config_descriptor(dev, 0, &config0) < 0)
        ftdi_error_return(-10, "libusb_get_config_descriptor() failed");
    cfg0 = config0->bConfigurationValue;
    libusb_free_config_descriptor (config0);
    libusb_detach_kernel_driver(ftdi->usb_dev, 0);
    if (libusb_get_configuration (ftdi->usb_dev, &cfg) < 0)
        ftdi_error_return(-12, "libusb_get_configuration () failed");
    if ((desc.bNumConfigurations > 0 && cfg != cfg0
        && libusb_set_configuration(ftdi->usb_dev, cfg0) < 0) {
     || libusb_claim_interface(ftdi->usb_dev, 0) < 0
     || libusb_control_transfer(ftdi->usb_dev, FTDI_DEVICE_OUT_REQTYPE, SIO_RESET_REQUEST, SIO_RESET_SIO, USB_INDEX, NULL, 0, WRITE_TIMEOUT) < 0)
        goto error;
    if (desc.bcdDevice == 0x400 || (desc.bcdDevice == 0x200 && desc.iSerialNumber == 0))
        ftdi->type = TYPE_BM;
    else if (desc.bcdDevice == 0x200)
        ftdi->type = TYPE_AM;
    else if (desc.bcdDevice == 0x500)
        ftdi->type = TYPE_2232C;
    else if (desc.bcdDevice == 0x600)
        ftdi->type = TYPE_R;
    else if (desc.bcdDevice == 0x700)
        ftdi->type = TYPE_2232H;
    else if (desc.bcdDevice == 0x800)
        ftdi->type = TYPE_4232H;
    else if (desc.bcdDevice == 0x900)
        ftdi->type = TYPE_232H;
    else if (desc.bcdDevice == 0x1000)
        ftdi->type = TYPE_230X;
    if (ftdi_set_baudrate (ftdi, 9600) != 0) {
    }
    return 0;
error:
    ftdi_usb_close_internal (ftdi);
    ftdi_error_return(-7, "set baudrate failed");
}
static struct ftdi_context *ftdi_new(void)
{
    struct ftdi_context * ftdi = (struct ftdi_context *)malloc(sizeof(struct ftdi_context));
printf("[%s:%d] funky version\n", __FUNCTION__, __LINE__);
    ftdi->usb_ctx = NULL;
    ftdi->usb_dev = NULL;
    ftdi->type = TYPE_BM;    /* chip type */
    if (libusb_init(&ftdi->usb_ctx) < 0)
        ftdi_error_return(NULL, "libusb_init() failed");
    return ftdi;
}
static void ftdi_set_usbdev (struct ftdi_context *ftdi, libusb_device_handle *usb)
{
    ftdi->usb_dev = usb;
}
