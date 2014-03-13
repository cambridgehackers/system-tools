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

#define USB_TIMEOUT    5000
#define ENDPOINT_IN     0x02
#define ENDPOINT_OUT    0x81
#define USB_INDEX       0
#define USB_CHUNKSIZE   4096
#define MPSSE_WRITE_NEG 0x01   /* Write TDI/DO on negative TCK/SK edge*/
#define MPSSE_BITMODE   0x02   /* Write bits, not bytes */
#define MPSSE_READ_NEG  0x04   /* Sample TDO/DI on negative TCK/SK edge */
#define MPSSE_LSB       0x08   /* LSB first */
#define MPSSE_DO_WRITE  0x10   /* Write TDI/DO */
#define MPSSE_DO_READ   0x20   /* Read TDO/DI */
#define MPSSE_WRITE_TMS 0x40   /* Write TMS/CS */
#define SET_BITS_LOW   0x80
#define SET_BITS_HIGH  0x82
#define LOOPBACK_END   0x85
//#define TCK_DIVISOR    0x86
#define DIS_DIV_5       0x8a
#define CLK_BYTES       0x8f
//#define DIV_VALUE(rate) (rate > 6000000)?0:((6000000/rate -1) > 0xffff)? 0xffff: (6000000/rate -1)
#define SEND_IMMEDIATE 0x87
#define SIO_RESET          0 /* Reset the port */
#define SIO_SET_BAUD_RATE  3 /* Set baud rate */
#define USB_OUT_REQTYPE (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT)
#define SIO_RESET_REQUEST             SIO_RESET
#define SIO_SET_BAUDRATE_REQUEST      SIO_SET_BAUD_RATE
#define SIO_SET_LATENCY_TIMER_REQUEST 0x09
#define SIO_SET_BITMODE_REQUEST       0x0B
#define SIO_RESET_SIO 0
#define SIO_RESET_PURGE_RX 1
#define SIO_RESET_PURGE_TX 2

struct ftdi_context {
    struct libusb_context *usb_ctx;
    struct libusb_device_handle *usb_dev;
    unsigned char readbuffer[USB_CHUNKSIZE];
};
struct ftdi_device_list {
    struct ftdi_device_list *next;
    struct libusb_device *dev;
};
#define ftdi_error_return(code, str) do {  \
        fprintf(stderr, str);          \
        return code;                       \
   } while(0);
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
static int ftdi_write_data(struct ftdi_context *ftdi, const unsigned char *buf, int size)
{
    int actual_length;
    if (libusb_bulk_transfer(ftdi->usb_dev, ENDPOINT_IN, (unsigned char *)buf, size, &actual_length, USB_TIMEOUT) < 0)
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
        ret = libusb_bulk_transfer (ftdi->usb_dev, ENDPOINT_OUT, ftdi->readbuffer, USB_CHUNKSIZE, &actual_length, USB_TIMEOUT);
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
static struct ftdi_context *ftdi_new(void)
{
    struct ftdi_context * ftdi = (struct ftdi_context *)malloc(sizeof(struct ftdi_context));
printf("[%s:%d] funky version\n", __FUNCTION__, __LINE__);
    ftdi->usb_ctx = NULL;
    ftdi->usb_dev = NULL;
    return ftdi;
}
static void ftdi_set_usbdev (struct ftdi_context *ftdi, libusb_device_handle *usb)
{
    ftdi->usb_dev = usb;
}
