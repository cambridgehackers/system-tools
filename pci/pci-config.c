/* pci-config-space.c: Read the PCI configuration space.

	Read the PCI configuration space using the Intel bus-bridge interface
	registers.  This bypasses the BIOS32 entry, and thus we are not assured
	of working on all systems.

	Copyright 1998-2001 by Donald Becker.
	This software may be used and distributed according to the terms of
	the GNU General Public License (GPL), incorporated herein by reference.
	Contact the author for use under other terms.

	The author may be reached as becker@scyld.com, or C/O
	 Scyld Computing Corporation
	 410 Severn Ave., Suite 210
	 Annapolis MD 21403

	Support and updates available at
	http://www.scyld.com/diag/index.html

	Common-sense licensing statement: Using any portion of this program in
	your own program means that you must give credit to the original author
	and release the resulting code under the GPL.
 */

static char *version_msg =
"pci-config.c:v2.02 1/8/2001 Donald Becker (becker@scyld.com)\n"
" http://www.scyld.com/diag/index.html\n";
static char *usage_msg =
"Usage: pci-config [-aDfSvVW] [-# <device_index>]\n";

static char *long_usage_msg ="\n"
"\n"
"  This program shows the contents of PCI configuration space.\n"
"  It reads the hardware registers, and thus must be run as 'root'.\n"
"\n"
"  Running this program with no options shows the installed PCI devices.\n"
"  Each line is prefixed by its index which may be used with -#<index>\n"
"  e.g. \"pci-config -#3\" to specify the device to operate on.\n"
"\n"
"  Commonly use options are\n"
"  -# <device-index>         Operate only on DEVICE-INDEX e.g  -#3\n"
"\n"
"  The operations on the selected device are\n"
"  -a  --show-addresses   Show PCI address registers.\n"
"  -S --sleep             Put device to sleep (ACPI D3 state)\n"
"  -W --wake              Wake a sleeping device (ACPI D0 state)\n"
"\n"
"  Less commonly used options are\n"
"  -B <bus>  --bus <bus>      Show only devices on BUS.\n"
"  -A <addr> --set-addresses  Set PCI address register 1 to the ADDR.\n"
"  -D  --debug                Show details of operations\n"
"  -f  --force                Override checks and perform the operation\n"
"  -u  --usage                Show this long usage message\n"
"  -v  --verbose              Verbose mode\n"
"  -V  --version              Display this program's version information\n"
"\n"
;

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#if defined(__linux__)  &&  __GNU_LIBRARY__ == 1
#include <asm/io.h>			/* Newer libraries use <sys/io.h> instead. */
#else
#include <sys/io.h>
#endif
#if !defined(__OPTIMIZE__)
#error You must compile this driver with "-O"!
#endif

struct option longopts[] = {
	{"show-addresses",	0, 0, 'a'},	/* Show PCI address registers. */
	{"set-addresses",	1, 0, 'A'},	/* Show PCI address registers. */
	{"bus",		1, 0, 'B'},			/* Show only devices on BUS. */
	{"debug",	0, 0, 'D'},			/* Increase debug level. */
	{"force",	0, 0, 'f'},			/* Force operation. */
	{"set-WOL",	0, 0, 'M'},			/* Set to Wake-On-LAN mode. */
	{"sleep",	0, 0, 'S'},			/* Put device to sleep (ACPI D3 state). */
	{"usage",	0, 0, 'u'},			/* Show the long usage message. */
	{"verbose",	0, 0, 'v'},			/* Verbose mode */
	{"version", 0, 0, 'V'},			/* Display version number */
	{"wake-on-lan", 0, 0, 'W'},		/* Wake (set to D0 state) the device. */
	{"device-index", 1, 0, '#'},	/* Operate only on device INDEX. */
	{ 0, 0, 0, 0 }
};

static int verbose=1, opt_a=0, opt_f=0, opt_wake=0, opt_set_WOL=0, debug=0;
static int opt_sleep = 0;
static long set_address = -1;

static void show_addr_config(unsigned char pci_bus, unsigned char pci_dev_fn);
static void show_ext_caps(unsigned int *cfg_space, unsigned char pci_bus,
						  unsigned char pci_dev_fn);
static void show_one_device(unsigned char pci_bus, unsigned char pci_dev_fn,
							int dev_num);

static void cyclone_WOL(int pci_bus, int pci_dev_fn, void *pci_config_space);
static void acpi_wake(unsigned char pci_bus, unsigned char pci_dev_fn,
					  void *config);
static void acpi_sleep(unsigned char bus, unsigned char devfn, void *pci_cfg);
static int dump_mem_region(long addr);

extern int pcibios_read_config_byte (unsigned char bus, unsigned char dev_fn,
				     unsigned char where, unsigned char *val);
int pcibios_read_config_word (unsigned char bus, unsigned char dev_fn,
							  unsigned char regnum, unsigned short *val);
extern int pcibios_read_config_dword (unsigned char bus, unsigned char dev_fn,
									  unsigned char regnum, unsigned int *val);
void pcibios_write_config_byte (unsigned char bus, unsigned char dev_fn,
								unsigned char regnum, unsigned char val);
void pcibios_write_config_word (unsigned char bus, unsigned char dev_fn,
								unsigned char regnum, unsigned short val);
void pcibios_write_config_dword (unsigned char bus, unsigned char dev_fn,
								unsigned char regnum, unsigned int val);


int main(int argc, char **argv)
{
	int pci_bus = 0, pci_dev_fn;
	int errflag = 0, show_version = 0;
	int c, longind, card_num = 0;
	int dev_num = 0;

	while ((c = getopt_long(argc, argv, "#:aA:b:B:DfMSuvVW",
							longopts, &longind))
		   != -1)
		switch (c) {
		case 'a': opt_a++;			break;
		case 'A': set_address = strtol(optarg, NULL, 16);	break;
		case 'b': printf("Setting bus to %s.\n", optarg);
		case 'B': pci_bus = strtol(optarg, NULL, 0); break;
		case 'D': debug++;			break;
		case 'f': opt_f++;			break;
		case 'M': opt_set_WOL++;	break;
		case 'S': opt_sleep++;		break;
		case 'u': printf("%s%s", usage_msg, long_usage_msg); return 0;
		case 'v': verbose++;		break;
		case 'V': show_version++;	break;
		case 'W': opt_wake++;		break;
		case '#': card_num = atoi(optarg); break;
		case '?':
			errflag++;
		}
	if (errflag) {
		fprintf(stderr, "%s%s", usage_msg, "  Use -u for more information.\n");
		return 3;
	}

	if (verbose)
		printf(version_msg);

	/* Get access to all of I/O space. */
	if (iopl(3) < 0) {
		perror("pci-config: iopl()");
		fprintf(stderr, "This program must be run as root.\n");
		return 2;
	}
	for (pci_dev_fn = 0; pci_dev_fn < 256; pci_dev_fn++) {
		/*unsigned char cb;*/
		unsigned int pci_id;

		pcibios_read_config_dword(pci_bus, pci_dev_fn, 0, &pci_id);
		if (pci_id == 0xffffffff)
			continue;
		dev_num++;
		if (card_num  == 0) {
			printf("Device #%d at bus %d device/function %d/%d, %8.8x.\n",
				   dev_num, pci_bus, pci_dev_fn>>3, pci_dev_fn&7, pci_id);
		} else if (card_num == dev_num) {
			show_one_device(pci_bus, pci_dev_fn, dev_num);
		}
		if ((pci_dev_fn & 7) == 0) {
			unsigned int cdw;
			pcibios_read_config_dword(pci_bus, pci_dev_fn, 3*4, &cdw);
			if ((cdw & 0x00800000) == 0)
				pci_dev_fn += 7;
		}
	}

	return 0;
}

static void show_one_device(unsigned char pci_bus, unsigned char pci_dev_fn,
							int dev_num)
{
	unsigned int config[64];
	int i;
	int pci_id;

	printf("Device #%d at bus %d device/function %d/%d.",
		   dev_num, pci_bus, pci_dev_fn>>3, pci_dev_fn&7);
	for (i = 0; i < 64; i++) {
		pcibios_read_config_dword(pci_bus, pci_dev_fn, i<<2, &config[i]);
		printf("%s%8.8x", i % 8 == 0  ?  "\n  " : " ", config[i]);
	}
	printf("\n");
	for (i = 0; i < 5; i++) {
		unsigned int pciaddr = config[4 + i];
		if (pciaddr)
			printf("  Base Address %d: %s at %8.8x.\n",
				   i, pciaddr & 1 ? "I/O" : "Memory", pciaddr & ~1);
	}
	if (set_address >= 0) {
		fprintf(stderr, "Setting PCI address register 1 to 0x%lx.\n",
				set_address);
		pcibios_write_config_dword(pci_bus, pci_dev_fn, 0x14, set_address);
		pcibios_write_config_dword(pci_bus, pci_dev_fn, 0x04, config[1] | 3);
	}
	if (opt_a)
		show_addr_config(pci_bus, pci_dev_fn);
	pci_id = config[0];
	if (config[1] & 0x00100000)
		show_ext_caps(config, pci_bus, pci_dev_fn);
	if (config[10]) {
		char *cis_addr_space[] = {"PCI configuration space", "BAR 0", "BAR 1", "BAR 2", "BAR 3", };
		int space = config[10] & 7;
		printf("  CardBus CIS pointer 0x%4.4x (%s), address %x.\n", config[10],
			   cis_addr_space[space], config[4 + (space-1)]);
		if (space > 0 && space < 4)
			dump_mem_region(config[4 + ((space-1))]);
	}
	if (opt_sleep)
		acpi_sleep(pci_bus, pci_dev_fn, config);
	if (opt_wake)
		acpi_wake(pci_bus, pci_dev_fn, config);
	if (opt_set_WOL)
		if (pci_id == 0x905510b7)
			cyclone_WOL(pci_bus, pci_dev_fn, config);
}

static void show_addr_config(unsigned char pci_bus, unsigned char pci_dev_fn)
{
	int i;
	unsigned int pciaddr, cdw;
	for (i = 0; i < 5; i++) {
		int cfg_i = 0x10 + (i<<2);
		pcibios_read_config_dword(pci_bus, pci_dev_fn, cfg_i, &pciaddr);
		pcibios_write_config_dword(pci_bus, pci_dev_fn, cfg_i, 0xffffffff);
		pcibios_read_config_dword(pci_bus, pci_dev_fn, cfg_i, &cdw);
		if (cdw == 0)
			break;
		printf("  Address %d %s at %8.8x, decoded bits are %8.8x.\n",
			   i, cdw & 1 ? "is I/O" : "memory", pciaddr & ~1, ~cdw);
		pcibios_write_config_dword(pci_bus, pci_dev_fn, cfg_i, pciaddr);
	}
	pcibios_read_config_dword(pci_bus, pci_dev_fn, 0x30, &pciaddr);
	pcibios_write_config_dword(pci_bus, pci_dev_fn, 0x30, 0xfffffffe);
	pcibios_read_config_dword(pci_bus, pci_dev_fn, 0x30, &cdw);
	pcibios_write_config_dword(pci_bus, pci_dev_fn, 0x30, pciaddr);
	printf("  BIOS ROM at %8.8x, decoded bits are %8.8x.\n", pciaddr, cdw);
	return;
}
static void show_ext_caps(unsigned int *cfg_space, unsigned char pci_bus,
						  unsigned char pci_dev_fn)
{
	unsigned char *pcfg = (void *)cfg_space;
	int cap_idx = cfg_space[13] & 0xff;

	printf("  Extended capabilities, first structure at offset 0x%x.\n",
		   cap_idx);
	for (; cap_idx; cap_idx = pcfg[cap_idx + 1]) {
		printf("  Extended PCI capability type %d at 0x%2.2x, next %d.\n",
			   pcfg[cap_idx], cap_idx, pcfg[cap_idx + 1]);
		if (pcfg[cap_idx] == 1) {
			printf("   Power management entry ver. %d: Capabilities %2.2x%2.2x"
				   ", Ctrl %2.2x%2.2x, Event %2.2x%2.2x.\n",
				   pcfg[cap_idx + 2] & 7,
				   pcfg[cap_idx + 3], pcfg[cap_idx + 2],
				   pcfg[cap_idx + 5], pcfg[cap_idx + 4],
				   pcfg[cap_idx + 7], pcfg[cap_idx + 6]);
			printf("   Power state D%d.\n", pcfg[cap_idx + 4] & 3);
		}
	}
}


static int acpi_find(unsigned char pci_bus, unsigned char pci_dev_fn,
					 	void *config)
{
	unsigned char *pcfg = (void *)config;
	if (pcfg[6] & 0x10) {
		int cap_idx = pcfg[0x34];

		printf("  Extended capabilities, first structure at offset 0x%x.\n",
			   cap_idx);
		for (; cap_idx; cap_idx = pcfg[cap_idx + 1]) {
			if (pcfg[cap_idx] == 1)
				return cap_idx;
		}
	}
	return 0;
}

static void acpi_wake(unsigned char pci_bus, unsigned char pci_dev_fn,
					  void *pci_config_space)
{
	unsigned char *config = pci_config_space;
	unsigned short *configw = pci_config_space;
	unsigned int *configdw = pci_config_space;
	int pwr_idx = acpi_find(pci_bus, pci_dev_fn, config);
	unsigned short pwr_command = configw[(pwr_idx + 4)>>1];
	int i;

	if (debug)
		printf("Power index is %#x.\n", pwr_idx);

	printf("  Waking up an ACPI device.  Currently powered %s, "
		   "I/O %#x IRQ %d.\n"
		   "  Updating the power state of %4.4x->%4.4x.\n",
		   pwr_command & 3 ? "down" : "up", configdw[0x10>>2], config[0x3c],
		   pwr_command, pwr_command & ~3);
	pcibios_write_config_word(pci_bus, pci_dev_fn, pwr_idx + 4,
							  pwr_command & ~3);
	/* Many devices must have their PCI register state restored when changing
	   from D3 state. */
	for (i = 0x10; i <= 0x20; i+=4)
		pcibios_write_config_dword(pci_bus, pci_dev_fn, i, configdw[i >> 2]);
	/* PCI_ROM_ADDRESS, interrupt line, cache line size, latency timer */
	pcibios_write_config_dword(pci_bus, pci_dev_fn, 0x30, configdw[0x30 >> 2]);
	pcibios_write_config_byte(pci_bus, pci_dev_fn, 0x3c, config[0x3c]);
	pcibios_write_config_byte(pci_bus, pci_dev_fn, 0x0c, config[0x0c]);
	pcibios_write_config_byte(pci_bus, pci_dev_fn, 0x0d, config[0x0d]);
	/* Finally, restore the command register. */
	pcibios_write_config_word(pci_bus, pci_dev_fn, 0x04, configw[4>>1]);
}

static void acpi_sleep(unsigned char bus, unsigned char devfn,
					   void *pci_config_space)
{
	unsigned short *configw = pci_config_space;
	int pwr_idx = acpi_find(bus, devfn, pci_config_space);
	unsigned short pwr_command = configw[(pwr_idx + 4)>>1];
	pcibios_write_config_word(bus, devfn, pwr_idx + 4, pwr_command | 0x0103);
}

/* Put the 3Com Cyclone e.g. 3c905B series into Wake On LAN mode. */
static void cyclone_WOL(int pci_bus, int pci_dev_fn, void *config)
{
	int pwr_idx = acpi_find(pci_bus, pci_dev_fn, config);
	unsigned short pwr_command = ((unsigned short *)config)[(pwr_idx + 4)>>1];
	long ioaddr = ((int *)config)[0x10];

	acpi_wake(pci_bus, pci_dev_fn, config);

	outw(0x801f, ioaddr + 0x0e); /* Set RxFilter to accept frames. */
	outw((1<<11) + 7, ioaddr + 0x0e);
	outw(7, ioaddr + 0x0c);
	printf("  Window 7 Power Management Event is %4.4x.\n",
		   inw(ioaddr + 0x0c));
	printf("  Changing the power state from %4.4x to 0103.\n", pwr_command);
	outw(0x2000, ioaddr + 0x0e); /* RxEnable. */
	pcibios_write_config_word(pci_bus, pci_dev_fn, 0xe0, pwr_command | 0x8103);
}


#define PCI_CONFIG_ADDR 0x0cf8
#define PCI_CONFIG_DATA 0x0cfc

int pcibios_read_config_byte (unsigned char bus, unsigned char dev_fn,
							  unsigned char regnum, unsigned char *val)
{
	outl(0x80000000 | (bus<<16) | (dev_fn << 8) | (regnum & 0xfc),
		 PCI_CONFIG_ADDR);
	*val = inb(PCI_CONFIG_DATA + (regnum & 3));
	return 0;
}
int pcibios_read_config_word (unsigned char bus, unsigned char dev_fn,
							  unsigned char regnum, unsigned short *val)
{
	outl(0x80000000 | (bus<<16) | (dev_fn << 8) | (regnum & 0xfc),
		 PCI_CONFIG_ADDR);
	*val = inw(PCI_CONFIG_DATA + (regnum & 2));
	return 0;
}
int pcibios_read_config_dword (unsigned char bus, unsigned char dev_fn,
							   unsigned char regnum, unsigned int *val)
{
	outl(0x80000000 | (bus<<16) | (dev_fn << 8) | (regnum & 0xfc),
		 PCI_CONFIG_ADDR);
	*val = inl(PCI_CONFIG_DATA);
	return 0;
}
void pcibios_write_config_byte (unsigned char bus, unsigned char dev_fn,
								unsigned char regnum, unsigned char val)
{
	outl(0x80000000 | (bus<<16) | (dev_fn << 8) | (regnum & 0xfc),
		 PCI_CONFIG_ADDR);
	outb(val, PCI_CONFIG_DATA + (regnum & 3));
	return;
}
void pcibios_write_config_word (unsigned char bus, unsigned char dev_fn,
								unsigned char regnum, unsigned short val)
{
	outl(0x80000000 | (bus<<16) | (dev_fn << 8) | (regnum & 0xfc),
		 PCI_CONFIG_ADDR);
	outw(val, PCI_CONFIG_DATA + (regnum & 2));
	return;
}
void pcibios_write_config_dword (unsigned char bus, unsigned char dev_fn,
								unsigned char regnum, unsigned int val)
{
	outl(0x80000000 | (bus<<16) | (dev_fn << 8) | (regnum & 0xfc),
		 PCI_CONFIG_ADDR);
	outl(val, PCI_CONFIG_DATA);
	return;
}


/* Map the board shared memory into our address space -- this code is
   a good example of non-kernel access to devices on the PCI bus. */
static int dump_mem_region(long addr)
{
	unsigned short *shared_mem;
	int i;
	int memfd = open("/dev/kmem", O_RDWR);

	if (memfd < 0) {
		perror("/dev/kmem (shared memory)");
		return 2;
	} else
	  printf("Opened /dev/kmem for PCI memory.\n");
	shared_mem = mmap(0, 0x8000, PROT_READ|PROT_WRITE,
					  MAP_SHARED, memfd, addr);
	printf("Shared memory at %#x (%p).\n", (int)addr, shared_mem);
	for (i = 0; i < 100; i++)
		printf(" %4.4x", shared_mem[i]);
	close(memfd);
	return 0;
}

/*
 * Local variables:
 *  compile-command: "cc -O -Wall -o pci-config pci-config.c"
 *  tab-width: 4
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
