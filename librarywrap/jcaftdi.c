
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

struct ftdi_context {
};
struct ftdi_transfer_control {
};
#define ftdi_error_return(code, str) do {  \
        fprintf(stderr, str);          \
        return code;                       \
   } while(0);
static void ftdi_deinit(struct ftdi_context *ftdi)
{
}
static int ftdi_transfer_data_done(struct ftdi_transfer_control *tc)
{
    return 0;
}
static int ftdi_write_data(struct ftdi_context *ftdi, const unsigned char *buf, int size)
{
    int actual_length;
    if (libusb_bulk_transfer(usbhandle, ENDPOINT_IN, (unsigned char *)buf, size, &actual_length, USB_TIMEOUT) < 0)
        ftdi_error_return(-1, "usb bulk write failed");
    return actual_length;
}
static struct ftdi_transfer_control *ftdi_write_data_submit(struct ftdi_context *ftdi, unsigned char *buf, int size)
{
    ftdi_write_data(ftdi, buf, size);
    return NULL;
}
static int ftdi_read_data(struct ftdi_context *ftdi, unsigned char *buf, int size)
{
    int offset = 0, ret;
    int actual_length = 1;
    do {
        ret = libusb_bulk_transfer (usbhandle, ENDPOINT_OUT, usbreadbuffer, USB_CHUNKSIZE, &actual_length, USB_TIMEOUT);
        if (ret < 0)
            ftdi_error_return(ret, "usb bulk read failed");
        actual_length -= 2;
    } while (actual_length == 0);
    memcpy (buf, usbreadbuffer+2, actual_length);
    if (actual_length != size) {
        printf("[%s:%d] bozo actual_length %d size %d\n", __FUNCTION__, __LINE__, actual_length, size);
        exit(-1);
        }
    return offset;
}
static struct ftdi_transfer_control *ftdi_read_data_submit(struct ftdi_context *ftdi, unsigned char *buf, int size)
{
    ftdi_read_data(ftdi, buf, size);
    return NULL;
}
static struct ftdi_context *ftdi_new(void)
{
printf("[%s:%d] funky version\n", __FUNCTION__, __LINE__);
    return (struct ftdi_context *)calloc(1, sizeof(struct ftdi_context));
}
static void ftdi_set_usbdev (struct ftdi_context *ftdi, libusb_device_handle *usb)
{
}
