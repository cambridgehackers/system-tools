/* Copyright 2013 Quanta Research Cambridge, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>

static unsigned char buf[1000000];

static void memdump(unsigned char *p, int len, char *title)
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

static uint32_t dumpword(char *name, int offset, int len)
{
    uint32_t temp = 0;
    memcpy(&temp, &buf[offset], len);
    if (temp)
        printf("%10s: %8x\n", name, temp);
    return temp;
}

int main(int argc, char *argv[])
{
    char *filename = "/sys/bus/pci/devices/0000:00:01.0/config";
    int fd = open(filename, O_RDONLY);
    int len = read(fd, buf, sizeof(buf));
    uint32_t temp;

    //memdump(buf, len, "DD");
    dumpword("VID", 0x0, 2);
    dumpword("DID", 0x2, 2);
    dumpword("PCICMD", 0x4, 2);
    dumpword("PCISTS", 0x6, 2);
    dumpword("RID", 0x8, 1);
    dumpword("CC", 0x9, 3);
    dumpword("CL", 0xC, 1);
    dumpword("HDR", 0xE, 1);
    dumpword("PBUSN", 0x18, 1);
    dumpword("SBUSN", 0x19, 1);
    dumpword("SUBUSN", 0x1A, 1);
    dumpword("IOBASE", 0x1C, 1);
    dumpword("IOLIMIT", 0x1D, 1);
    dumpword("SSTS", 0x1E, 2);
    dumpword("MBASE", 0x20, 2);
    dumpword("MLIMIT", 0x22, 2);
    dumpword("PMBASE", 0x24, 2);
    dumpword("PMLIMIT", 0x26, 2);
    dumpword("PMBASEU", 0x28, 4);
    dumpword("PMLIMITU", 0x2C, 4);
    dumpword("CAPPTR", 0x34, 1);
    dumpword("INTRLINE", 0x3C, 1);
    dumpword("INTRPIN", 0x3D, 1);
    dumpword("BCTRL", 0x3E, 2);
    dumpword("PM", 0x80, 4);
    dumpword("PM", 0x84, 4);
    dumpword("SS", 0x88, 4);
    dumpword("SS", 0x8C, 4);
    dumpword("MSI", 0x90, 2);
    dumpword("MC", 0x92, 2);
    dumpword("MA", 0x94, 4);
    dumpword("MD", 0x98, 2);
    dumpword("PEG0", 0xA0, 2);
    dumpword("PEG2", 0xA2, 2);
    dumpword("DCAP", 0xA4, 4);
    dumpword("DCTL", 0xA8, 2);
    temp = dumpword("DSTS", 0xAA, 2);
    if (temp & 1)
        printf("DSTS: CED Correctable Error Detected\n");
    dumpword("LCTL", 0xB0, 2);
    dumpword("LSTS", 0xB2, 2);
    dumpword("SLOTCAP", 0xB4, 4);
    dumpword("SLOTCTL", 0xB8, 2);
    temp = dumpword("SLOTSTS", 0xBA, 2);
    if (temp & 8)
        printf("SLOTSTS: PDC Presence Detect Changed\n");
    dumpword("RCTL", 0xBC, 2);
    dumpword("RSTS", 0xC0, 4);
    dumpword("DCAP2", 0xC4, 4);
    temp = dumpword("DCTL2", 0xC8, 2);
    if (!(temp & 0x400))
        printf("DCTL2: LTREN Latency Tolerance Reporting Mechanism Disabled\n");
    dumpword("LCTL2", 0xD0, 2);
    dumpword("LSTS2", 0xD2, 2);
    dumpword("PVCCAP1", 0x104, 4);
    dumpword("PVCCAP2", 0x108, 4);
    dumpword("PVCCTL", 0x10C, 2);
    dumpword("VC0RCAP", 0x110, 4);
    dumpword("VC0RCTL", 0x114, 4);
    temp = dumpword("VC0RSTS", 0x11A, 4);
    if (temp & 2)
        printf("VC0RSTS: VC0NP negotiation pending\n");
    return 0;
}
