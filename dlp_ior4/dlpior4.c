// Copyright (c) 2014 Quanta Research Cambridge, Inc.
// Original author: John Ankcorn

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <zlib.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <libusb.h>

#define USB_TIMEOUT     5000
#define ENDPOINT_IN     0x02
#define ENDPOINT_OUT    0x81
#define USB_CHUNKSIZE   4096
#define USB_INDEX          0

#define USBSIO_RESET                     0 /* Reset the port */
#define USBSIO_RESET_PURGE_RX            1
#define USBSIO_RESET_PURGE_TX            2
#define USBSIO_SET_BAUD_RATE             3 /* Set baud rate */
#define USBSIO_SET_LATENCY_TIMER_REQUEST 9
#define USBSIO_SET_BITMODE_REQUEST       11
#define MAX_ITEM_LENGTH 2000
#define MAX_USB_DEVICECOUNT 100

typedef struct {
    void          *dev;
    int           idProduct;
    int           bcdDevice;
    int           bNumConfigurations;
    unsigned char iSerialNumber[64], iManufacturer[64], iProduct[128];
} USB_INFO;

static USB_INFO usbinfo_array[MAX_USB_DEVICECOUNT];
static int usbinfo_array_index;
static libusb_device **device_list;

#if defined(USE_TRACING)
static int trace = 1;
#else
static int trace;
#endif

static int logging;
static libusb_device_handle *usbhandle = NULL;
static struct libusb_context *usb_context;
static uint8_t usbreadbuffer[USB_CHUNKSIZE];

static void memdump(const uint8_t *p, int len, char *title)
{
int i;

    i = 0;
    while (len > 0) {
        if (title && !(i & 0xf)) {
            if (i > 0)
                printf("\n");
            printf("%s: ",title);
        }
        printf("0x%02x, ", *p++);
        i++;
        len--;
    }
    if (title)
        printf("\n");
}

int usb_write_data(const unsigned char *buf, int size)
{
    int actual_length;
    if (logging)
        memdump(buf, size, "WRITE");
    if (libusb_bulk_transfer(usbhandle, ENDPOINT_IN, (unsigned char *)buf, size, &actual_length, USB_TIMEOUT) < 0)
        printf( "usb bulk write failed");
    return actual_length;
}
int usb_read_data(unsigned char *buf, int size)
{
    int actual_length = 1;
    do {
        int ret = libusb_bulk_transfer (usbhandle, ENDPOINT_OUT, usbreadbuffer, USB_CHUNKSIZE, &actual_length, USB_TIMEOUT);
        if (ret < 0)
            printf( "usb bulk read failed");
        actual_length -= 2;
    } while (actual_length == 0);
    return actual_length;
}

/*
 * USB interface
 */
static USB_INFO *usb_init(void)
{
    int i = 0;
    libusb_device *dev;
#define UDESC(A) libusb_get_string_descriptor_ascii(usbhandle, desc.A, \
     usbinfo_array[usbinfo_array_index].A, sizeof(usbinfo_array[usbinfo_array_index].A))

    /*
     * Locate USB interface for JTAG
     */
    if (libusb_init(&usb_context) < 0
     || libusb_get_device_list(usb_context, &device_list) < 0) {
        printf("libusb_init failed\n");
        exit(-1);
    }
    while ((dev = device_list[i++]) ) {
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(dev, &desc) < 0)
            break;
        if ( desc.idVendor == 0x403 && (desc.idProduct == 0x6001 || desc.idProduct == 0x6010
         || desc.idProduct == 0x6011 || desc.idProduct == 0x6014)) {
            usbinfo_array[usbinfo_array_index].dev = dev;
            usbinfo_array[usbinfo_array_index].idProduct = desc.idProduct;
            usbinfo_array[usbinfo_array_index].bcdDevice = desc.bcdDevice;
            usbinfo_array[usbinfo_array_index].bNumConfigurations = desc.bNumConfigurations;
            if (libusb_open(dev, &usbhandle) < 0
             || UDESC(iManufacturer) < 0 || UDESC(iProduct) < 0 || UDESC(iSerialNumber) < 0) {
                printf("Error getting USB device attributes\n");
                exit(-1);
            }
            libusb_close (usbhandle);
            usbinfo_array_index++;
        }
    }
    return usbinfo_array;
}

static void usb_open(int device_index)
{
    int cfg, baudrate = 9600;
    static const char frac_code[8] = {0, 3, 2, 4, 1, 5, 6, 7};
    int best_divisor = 12000000*8 / baudrate;
    unsigned long encdiv = (best_divisor >> 3) | (frac_code[best_divisor & 0x7] << 14);
    struct libusb_config_descriptor *config_descrip;

    libusb_open(usbinfo_array[device_index].dev, &usbhandle);
    if (libusb_get_config_descriptor(usbinfo_array[device_index].dev, 0, &config_descrip) < 0)
        goto error;
    int configv = config_descrip->bConfigurationValue;
    libusb_free_config_descriptor (config_descrip);
    libusb_detach_kernel_driver(usbhandle, 0);
#define USBCTRL(A,B,C) \
     libusb_control_transfer(usbhandle, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE \
           | LIBUSB_ENDPOINT_OUT, (A), (B), (C) | USB_INDEX, NULL, 0, USB_TIMEOUT)

    if (libusb_get_configuration (usbhandle, &cfg) < 0
     || (usbinfo_array[device_index].bNumConfigurations > 0 && cfg != configv && libusb_set_configuration(usbhandle, configv) < 0)
     || libusb_claim_interface(usbhandle, 0) < 0
     || USBCTRL(USBSIO_RESET, USBSIO_RESET, 0) < 0
     || USBCTRL(USBSIO_SET_BAUD_RATE, (encdiv | 0x20000) & 0xFFFF, ((encdiv >> 8) & 0xFF00)) < 0
     || USBCTRL(USBSIO_SET_LATENCY_TIMER_REQUEST, 255, 0) < 0
     || USBCTRL(USBSIO_SET_BITMODE_REQUEST, 0, 0) < 0
     || USBCTRL(USBSIO_SET_BITMODE_REQUEST, 2 << 8, 0) < 0
     || USBCTRL(USBSIO_RESET, USBSIO_RESET_PURGE_RX, 0) < 0
     || USBCTRL(USBSIO_RESET, USBSIO_RESET_PURGE_TX, 0) < 0)
        goto error;
    return;
error:
    printf("Error opening usb interface\n");
    exit(-1);
}

int main(int argc, char **argv)
{
    int i, rflag = 0, lflag = 0;
    const char *serialno = NULL;

    opterr = 0;
    while ((i = getopt (argc, argv, "trls:")) != -1)
        switch (i) {
        case 't':
            trace = 1;
            break;
        case 'r':
            rflag = 1;
            break;
        case 'l':
            lflag = 1;
            break;
        case 's':
            serialno = optarg;
            break;
        default:
            goto usage;
        }

    /*
     * Initialize USB
     */
    USB_INFO *uinfo = usb_init();   /*** Initialize USB interface ***/
    int usb_index = 0;
    for (i = 0; uinfo[i].dev; i++) {
        printf("[%s] %s:%s:%s; bcd:%x", __FUNCTION__, uinfo[i].iManufacturer,
            uinfo[i].iProduct, uinfo[i].iSerialNumber, uinfo[i].bcdDevice);
        if (lflag) {
            usb_open(i);  /*** Generic initialization of FTDI chip ***/
            libusb_close (usbhandle);
            printf("; IDCODE:");
        }
        printf("\n");
    }
    if (lflag)
        exit(0);
    while (1) {
        if (!uinfo[usb_index].dev) {
            printf("Can't find usable usb interface\n");
            exit(-1);
        }
        if (!serialno || !strcmp(serialno, (char *)uinfo[usb_index].iSerialNumber))
            break;
        usb_index++;
    }
    if (optind != argc - 1) {
usage:
        printf("%s: [ -l ] [ -t ] [ -s <serialno> ] [ -r ] <filename>\n", argv[0]);
        exit(1);
    }

    /*
     * Set JTAG clock speed and GPIO pins for our i/f
     */
    usb_open(usb_index);          /*** Generic initialization of FTDI chip ***/

    /*
     * Cleanup and free USB device
     */
    libusb_close (usbhandle);
    libusb_free_device_list(device_list,1);
    libusb_exit(usb_context);
    execlp("/usr/local/bin/pciescanportal", "arg", (char *)NULL); /* rescan pci bus to discover device */
    return 0;
}
