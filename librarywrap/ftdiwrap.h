#include <stdio.h>
#include <dlfcn.h>

static FILE *logfile;
static void openlogfile(void)
{
if (!logfile)
    logfile = fopen("/tmp/xx.logfile", "w");
}
int ftdi_read_eeprom(struct ftdi_context *ftdi)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_read_eeprom");
    }
    /////////////////////////////////////////
    {
    //fprintf(logfile, "%s(%s);\n", __FUNCTION__, translate_context(ftdi));
    }
    /////////////////////////////////////////
    retval = real_func(ftdi);
    //if (retval)
        //fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_eeprom_initdefaults(struct ftdi_context *ftdi, char * manufacturer, char *product, char * serial)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, char * manufacturer, char *product, char * serial) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_eeprom_initdefaults");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, manufacturer, product, serial);
    return retval;
}

int ftdi_usb_open_dev(struct ftdi_context *ftdi, struct libusb_device *dev)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, struct libusb_device *dev) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_usb_open_dev");
    }
    /////////////////////////////////////////
    {
    //fprintf(logfile, "%s(%s, %p);\n", __FUNCTION__, translate_context(ftdi), dev);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, dev);
    //if (retval)
        //fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_write_data_set_chunksize(struct ftdi_context *ftdi, unsigned int chunksize)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, unsigned int chunksize) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_write_data_set_chunksize");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, chunksize);
    return retval;
}

int ftdi_usb_get_strings(struct ftdi_context *ftdi, struct libusb_device *dev, char * manufacturer, int mnf_len, char * description, int desc_len, char * serial, int serial_len)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, struct libusb_device *dev, char * manufacturer, int mnf_len, char * description, int desc_len, char * serial, int serial_len) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_usb_get_strings");
    }
    /////////////////////////////////////////
    {
//int ftdi_usb_get_strings(struct ftdi_context *ftdi, struct libusb_device *dev, char * manufacturer, int mnf_len, char * description, int desc_len, char * serial, int serial_len)
    //fprintf(logfile, "%s(%s, %p, %s, %s, %s);\n", __FUNCTION__, translate_context(ftdi), dev,
        //translate_buffer(manufacturer, mnf_len), translate_buffer(description, desc_len), translate_buffer(serial, serial_len));
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, dev, manufacturer, mnf_len, description, desc_len, serial, serial_len);
    //fprintf(logfile, "//%s: ret %d man=%s\n//      desc=%s\n//      serial=%s\n", __FUNCTION__, retval, manufacturer, description, serial);
    return retval;
}

int ftdi_eeprom_decode(struct ftdi_context *ftdi, int verbose)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, int verbose) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_eeprom_decode");
    }
    /////////////////////////////////////////
    {
    //fprintf(logfile, "%s(%s, %d);\n", __FUNCTION__, translate_context(ftdi), verbose);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, verbose);
    //if (retval)
        //fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_set_bitmode(struct ftdi_context *ftdi, unsigned char bitmask, unsigned char mode)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, unsigned char bitmask, unsigned char mode) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_set_bitmode");
    }
    /////////////////////////////////////////
    {
    //fprintf(logfile, "%s(%s, 0x%x, 0x%x);\n", __FUNCTION__, translate_context(ftdi), bitmask, mode);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, bitmask, mode);
    //if (retval)
        //fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_usb_purge_rx_buffer(struct ftdi_context *ftdi)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_usb_purge_rx_buffer");
    }
    /////////////////////////////////////////
    {
    //fprintf(logfile, "%s(%s);\n", __FUNCTION__, translate_context(ftdi));
    }
    /////////////////////////////////////////
    retval = real_func(ftdi);
    //if (retval)
        //fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_usb_close(struct ftdi_context *ftdi)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_usb_close");
    }
    /////////////////////////////////////////
    {
    //fprintf(logfile, "%s(%s);\n", __FUNCTION__, translate_context(ftdi));
    }
    /////////////////////////////////////////
    retval = real_func(ftdi);
    //if (retval)
        //fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_get_latency_timer(struct ftdi_context *ftdi, unsigned char *latency)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, unsigned char *latency) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_get_latency_timer");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, latency);
    return retval;
}

int ftdi_get_eeprom_buf(struct ftdi_context *ftdi, unsigned char * buf, int size)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, unsigned char * buf, int size) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_get_eeprom_buf");
    }
    /////////////////////////////////////////
    {
    //fprintf(logfile, "%s(%s, %p, %d);\n", __FUNCTION__, translate_context(ftdi), buf, size);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, buf, size);
    if (retval)
        fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

void ftdi_set_usbdev(struct ftdi_context *ftdi, struct libusb_device_handle *usbdev)
{
    static void *dlopen_ptr = NULL;
    static void (*real_func)(struct ftdi_context *ftdi, struct libusb_device_handle *usbdev) = NULL;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_set_usbdev");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    real_func(ftdi, usbdev);
}

int ftdi_write_data_get_chunksize(struct ftdi_context *ftdi, unsigned int *chunksize)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, unsigned int *chunksize) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_write_data_get_chunksize");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, chunksize);
    return retval;
}

int ftdi_readstream(struct ftdi_context *ftdi, FTDIStreamCallback *callback, void *userdata, int packetsPerTransfer, int numTransfers)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, FTDIStreamCallback *callback, void *userdata, int packetsPerTransfer, int numTransfers) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_readstream");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, callback, userdata, packetsPerTransfer, numTransfers);
    return retval;
}

int ftdi_write_data(struct ftdi_context *ftdi, const unsigned char *buf, int size)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, const unsigned char *buf, int size) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_write_data");
    }
    /////////////////////////////////////////
    {
    writedata(0, buf, size);
    //fprintf(logfile, "%s(%s, %s);\n", __FUNCTION__, translate_context(ftdi), writedata(0, buf, size));
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, buf, size);
    //if (retval != size)
        //fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_read_eeprom_location(struct ftdi_context *ftdi, int eeprom_addr, unsigned short *eeprom_val)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, int eeprom_addr, unsigned short *eeprom_val) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_read_eeprom_location");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, eeprom_addr, eeprom_val);
    return retval;
}

int ftdi_read_data(struct ftdi_context *ftdi, unsigned char *buf, int size)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, unsigned char *buf, int size) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_read_data");
    }
    /////////////////////////////////////////
    {
//int ftdi_read_data(struct ftdi_context *ftdi, unsigned char *buf, int size)
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, buf, size);
    readdata(buf, size);
    //fprintf(logfile, "%s(%s, %s);\n", __FUNCTION__, translate_context(ftdi), readdata(buf, size));
    if (retval != size)
        fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_set_event_char(struct ftdi_context *ftdi, unsigned char eventch, unsigned char enable)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, unsigned char eventch, unsigned char enable) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_set_event_char");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, eventch, enable);
    return retval;
}

int ftdi_usb_reset(struct ftdi_context *ftdi)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_usb_reset");
    }
    /////////////////////////////////////////
    {
    //fprintf(logfile, "%s(%s);\n", __FUNCTION__, translate_context(ftdi));
    }
    /////////////////////////////////////////
    retval = real_func(ftdi);
    if (retval)
        fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_set_eeprom_value(struct ftdi_context *ftdi, enum ftdi_eeprom_value value_name, int  value)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, enum ftdi_eeprom_value value_name, int  value) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_set_eeprom_value");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, value_name, value);
    return retval;
}

int ftdi_usb_open_desc(struct ftdi_context *ftdi, int vendor, int product, const char* description, const char* serial)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, int vendor, int product, const char* description, const char* serial) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_usb_open_desc");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, vendor, product, description, serial);
    return retval;
}

struct ftdi_context *ftdi_new(void)
{
    static void *dlopen_ptr = NULL;
    static struct ftdi_context *(*real_func)(void) = NULL;
    struct ftdi_context * retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return NULL;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_new");
    }
    /////////////////////////////////////////
    {
    //fprintf(logfile, "//%s() starting\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func();
    fprintf(logfile, "//%p\n%s = %s();\n", retval, translate_context(retval), __FUNCTION__);
    if (!master_ftdi)
        master_ftdi = retval;
    if (datafile_fd < 0)
        datafile_fd = creat("/tmp/xx.datafile", 0666);
    return retval;
}

int ftdi_transfer_data_done(struct ftdi_transfer_control *tc)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_transfer_control *tc) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_transfer_data_done");
    }
    /////////////////////////////////////////
    {
    }
    /////////////////////////////////////////
    retval = real_func(tc);
    if (tc == read_data_submit_control) {
        if (retval != read_data_len)
            fprintf(logfile, "%s(%p) = %d\n", __FUNCTION__, tc, retval);
        readdata(read_data_buffer, read_data_len);
        //fprintf(logfile, "check_ftdi_read_data_submit(%s, %s);\n", translate_context(read_data_context), readdata(read_data_buffer, read_data_len));
        read_data_submit_control = NULL;
    }
    else if (tc == write_data_submit_control) {
        write_data_submit_control = NULL;
        //if (retval != write_data_len)
            //fprintf(logfile, "%s(%p) = %d\n", __FUNCTION__, tc, retval);
    }
    else
        fprintf(logfile, "%s(%p) = %d\n", __FUNCTION__, tc, retval);
    return retval;
}

struct ftdi_version_info ftdi_get_library_version(void)
{
    static void *dlopen_ptr = NULL;
    static struct ftdi_version_info (*real_func)(void) = NULL;
    struct ftdi_version_info retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_get_library_version");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func();
    return retval;
}

int ftdi_usb_open_desc_index(struct ftdi_context *ftdi, int vendor, int product, const char* description, const char* serial, unsigned int index)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, int vendor, int product, const char* description, const char* serial, unsigned int index) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_usb_open_desc_index");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, vendor, product, description, serial, index);
    return retval;
}

int ftdi_init(struct ftdi_context *ftdi)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_init");
    }
    /////////////////////////////////////////
    {
    //fprintf(logfile, "\n%s(%s);\n", __FUNCTION__, translate_context(ftdi));
    }
    /////////////////////////////////////////
    retval = real_func(ftdi);
    return retval;
}

int ftdi_write_eeprom(struct ftdi_context *ftdi)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_write_eeprom");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi);
    return retval;
}

void ftdi_list_free2(struct ftdi_device_list *devlist)
{
    static void *dlopen_ptr = NULL;
    static void (*real_func)(struct ftdi_device_list *devlist) = NULL;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_list_free2");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    real_func(devlist);
}

int ftdi_usb_find_all(struct ftdi_context *ftdi, struct ftdi_device_list **devlist, int vendor, int product)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, struct ftdi_device_list **devlist, int vendor, int product) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_usb_find_all");
    }
    /////////////////////////////////////////
    {
//int ftdi_usb_find_all(struct ftdi_context *ftdi, struct ftdi_device_list **devlist, int vendor, int product)
    //fprintf(logfile, "%s(%s, %p, 0x%x, 0x%x);\n", __FUNCTION__, translate_context(ftdi), devlist, vendor, product);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, devlist, vendor, product);
    //if (retval)
        //fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_setflowctrl(struct ftdi_context *ftdi, int flowctrl)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, int flowctrl) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_setflowctrl");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, flowctrl);
    return retval;
}

int ftdi_set_line_property2(struct ftdi_context *ftdi, enum ftdi_bits_type bits, enum ftdi_stopbits_type sbit, enum ftdi_parity_type parity, enum ftdi_break_type break_type)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, enum ftdi_bits_type bits, enum ftdi_stopbits_type sbit, enum ftdi_parity_type parity, enum ftdi_break_type break_type) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_set_line_property2");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, bits, sbit, parity, break_type);
    return retval;
}

int ftdi_poll_modem_status(struct ftdi_context *ftdi, unsigned short *status)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, unsigned short *status) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_poll_modem_status");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, status);
    return retval;
}

int ftdi_set_error_char(struct ftdi_context *ftdi, unsigned char errorch, unsigned char enable)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, unsigned char errorch, unsigned char enable) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_set_error_char");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, errorch, enable);
    return retval;
}

int ftdi_setrts(struct ftdi_context *ftdi, int state)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, int state) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_setrts");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, state);
    return retval;
}

int ftdi_set_latency_timer(struct ftdi_context *ftdi, unsigned char latency)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, unsigned char latency) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_set_latency_timer");
    }
    /////////////////////////////////////////
    {
    //fprintf(logfile, "%s(%s, %d);\n", __FUNCTION__, translate_context(ftdi), latency);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, latency);
    //if (retval)
        //fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_read_data_set_chunksize(struct ftdi_context *ftdi, unsigned int chunksize)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, unsigned int chunksize) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_read_data_set_chunksize");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "%s(%s, %d);\n", __FUNCTION__, translate_context(ftdi), chunksize);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, chunksize);
    if (retval)
        fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_usb_purge_buffers(struct ftdi_context *ftdi)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_usb_purge_buffers");
    }
    /////////////////////////////////////////
    {
    //fprintf(logfile, "%s(%s);\n", __FUNCTION__, translate_context(ftdi));
    }
    /////////////////////////////////////////
    retval = real_func(ftdi);
    //if (retval)
        //fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_setdtr(struct ftdi_context *ftdi, int state)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, int state) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_setdtr");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, state);
    return retval;
}

int ftdi_read_data_get_chunksize(struct ftdi_context *ftdi, unsigned int *chunksize)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, unsigned int *chunksize) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_read_data_get_chunksize");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, chunksize);
    return retval;
}

int ftdi_write_eeprom_location(struct ftdi_context *ftdi, int eeprom_addr, unsigned short eeprom_val)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, int eeprom_addr, unsigned short eeprom_val) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_write_eeprom_location");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, eeprom_addr, eeprom_val);
    return retval;
}

char *ftdi_get_error_string(struct ftdi_context *ftdi)
{
    static void *dlopen_ptr = NULL;
    static char *(*real_func)(struct ftdi_context *ftdi) = NULL;
    char * retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return NULL;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_get_error_string");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi);
    return retval;
}

int ftdi_erase_eeprom(struct ftdi_context *ftdi)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_erase_eeprom");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi);
    return retval;
}

int ftdi_usb_open_string(struct ftdi_context *ftdi, const char* description)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, const char* description) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_usb_open_string");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, description);
    return retval;
}

struct ftdi_transfer_control *ftdi_read_data_submit(struct ftdi_context *ftdi, unsigned char *buf, int size)
{
    static void *dlopen_ptr = NULL;
    static struct ftdi_transfer_control *(*real_func)(struct ftdi_context *ftdi, unsigned char *buf, int size) = NULL;
    struct ftdi_transfer_control * retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return NULL;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_read_data_submit");
    }
    /////////////////////////////////////////
    {
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, buf, size);
//struct ftdi_transfer_control *ftdi_read_data_submit(struct ftdi_context *ftdi, unsigned char *buf, int size)
    //fprintf(logfile, "%s(%s, %p, %d) = %p\n", __FUNCTION__, translate_context(ftdi), buf, size, retval);
    read_data_context = ftdi;
    read_data_submit_control = retval;
    read_data_buffer = buf;
    read_data_len = size;
    return retval;
}

int ftdi_setdtr_rts(struct ftdi_context *ftdi, int dtr, int rts)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, int dtr, int rts) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_setdtr_rts");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, dtr, rts);
    return retval;
}

int ftdi_usb_purge_tx_buffer(struct ftdi_context *ftdi)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_usb_purge_tx_buffer");
    }
    /////////////////////////////////////////
    {
    //fprintf(logfile, "%s(%s);\n", __FUNCTION__, translate_context(ftdi));
    }
    /////////////////////////////////////////
    retval = real_func(ftdi);
    if (retval)
        fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_get_eeprom_value(struct ftdi_context *ftdi, enum ftdi_eeprom_value value_name, int* value)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, enum ftdi_eeprom_value value_name, int* value) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_get_eeprom_value");
    }
    /////////////////////////////////////////
    {
//int ftdi_get_eeprom_value(struct ftdi_context *ftdi, enum ftdi_eeprom_value value_name, int* value)
    //fprintf(logfile, "%s(%s, %d, %p);\n", __FUNCTION__, translate_context(ftdi), value_name, value);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, value_name, value);
    //if (retval)
        //fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

void ftdi_deinit(struct ftdi_context *ftdi)
{
    static void *dlopen_ptr = NULL;
    static void (*real_func)(struct ftdi_context *ftdi) = NULL;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_deinit");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "%s(%s);\n", __FUNCTION__, translate_context(ftdi));
    if (master_ftdi == ftdi)
        final_dump();
    }
    /////////////////////////////////////////
    real_func(ftdi);
}

int ftdi_read_pins(struct ftdi_context *ftdi, unsigned char *pins)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, unsigned char *pins) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_read_pins");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, pins);
    return retval;
}

void ftdi_list_free(struct ftdi_device_list **devlist)
{
    static void *dlopen_ptr = NULL;
    static void (*real_func)(struct ftdi_device_list **devlist) = NULL;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_list_free");
    }
    /////////////////////////////////////////
    {
//void ftdi_list_free(struct ftdi_device_list **devlist)
    //fprintf(logfile, "%s(%p);\n", __FUNCTION__, devlist);
    }
    /////////////////////////////////////////
    real_func(devlist);
}

struct ftdi_transfer_control *ftdi_write_data_submit(struct ftdi_context *ftdi, unsigned char *buf, int size)
{
    static void *dlopen_ptr = NULL;
    static struct ftdi_transfer_control *(*real_func)(struct ftdi_context *ftdi, unsigned char *buf, int size) = NULL;
    struct ftdi_transfer_control * retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return NULL;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_write_data_submit");
    }
    /////////////////////////////////////////
    {
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, buf, size);
    char *p = writedata(1, buf, size);
    //if (accum < ACCUM_LIMIT)
        //fprintf(logfile, "%s(%s, %s);\n", __FUNCTION__, translate_context(ftdi), p);
    write_data_submit_control = retval;
    write_data_len = size;
    return retval;
}

int ftdi_set_eeprom_buf(struct ftdi_context *ftdi, const unsigned char * buf, int size)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, const unsigned char * buf, int size) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_set_eeprom_buf");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, buf, size);
    return retval;
}

void ftdi_free(struct ftdi_context *ftdi)
{
    static void *dlopen_ptr = NULL;
    static void (*real_func)(struct ftdi_context *ftdi) = NULL;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_free");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "%s(%s);\n", __FUNCTION__, translate_context(ftdi));
    }
    /////////////////////////////////////////
    real_func(ftdi);
}

int ftdi_set_baudrate(struct ftdi_context *ftdi, int baudrate)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, int baudrate) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_set_baudrate");
    }
    /////////////////////////////////////////
    {
    //fprintf(logfile, "%s(%s, %d);\n", __FUNCTION__, translate_context(ftdi), baudrate);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, baudrate);
    //if (retval)
        //fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_usb_open(struct ftdi_context *ftdi, int vendor, int product)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, int vendor, int product) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_usb_open");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, vendor, product);
    return retval;
}

int ftdi_read_chipid(struct ftdi_context *ftdi, unsigned int *chipid)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, unsigned int *chipid) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_read_chipid");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, chipid);
    return retval;
}

int ftdi_disable_bitbang(struct ftdi_context *ftdi)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_disable_bitbang");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi);
    return retval;
}

int ftdi_eeprom_build(struct ftdi_context *ftdi)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_eeprom_build");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi);
    return retval;
}

int ftdi_set_line_property(struct ftdi_context *ftdi, enum ftdi_bits_type bits, enum ftdi_stopbits_type sbit, enum ftdi_parity_type parity)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, enum ftdi_bits_type bits, enum ftdi_stopbits_type sbit, enum ftdi_parity_type parity) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_set_line_property");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, bits, sbit, parity);
    return retval;
}

int ftdi_set_interface(struct ftdi_context *ftdi, enum ftdi_interface interface)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, enum ftdi_interface interface) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_set_interface");
    }
    /////////////////////////////////////////
    {
    //fprintf(logfile, "%s(%s, %d);\n", __FUNCTION__, translate_context(ftdi), interface);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, interface);
    //if (retval)
        //fprintf(logfile, "%s: returncode %d\n", __FUNCTION__, retval);
    return retval;
}

int ftdi_eeprom_set_strings(struct ftdi_context *ftdi, char * manufacturer, char * product, char * serial)
{
    static void *dlopen_ptr = NULL;
    static int (*real_func)(struct ftdi_context *ftdi, char * manufacturer, char * product, char * serial) = NULL;
    int retval;
    if (!dlopen_ptr) {
        openlogfile();
        if (!(dlopen_ptr = dlopen("/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0", RTLD_LAZY))) {
            fprintf(stderr, "Failed to dlopen /home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0, error %s\n", dlerror());
            return -1;
        }
        real_func = dlsym(dlopen_ptr, "ftdi_eeprom_set_strings");
    }
    /////////////////////////////////////////
    {
    fprintf(logfile, "[%s] called\n", __FUNCTION__);
    }
    /////////////////////////////////////////
    retval = real_func(ftdi, manufacturer, product, serial);
    return retval;
}

