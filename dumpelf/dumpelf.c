/*
 * Copyright 2005-2007 Gentoo Foundation
 * Distributed under the terms of the GNU General Public License v2
 * $Header: /var/cvsroot/gentoo-projects/pax-utils/dumpelf.c,v 1.31 2011/10/13 04:48:44 vapier Exp $
 *
 * Copyright 2005-2007 Ned Ludd        - <solar@gentoo.org>
 * Copyright 2005-2007 Mike Frysinger  - <vapier@gentoo.org>
 */

static const char rcsid[] = "$Id: dumpelf.c,v 1.31 2011/10/13 04:48:44 vapier Exp $";
const char argv0[] = "dumpelf";

##include "paxinc.h"

/* prototypes */
static void dumpelf(const char *filename, long file_cnt);
static void dump_ehdr(elfobj *elf, void *ehdr);
static void dump_phdr(elfobj *elf, void *phdr, long phdr_cnt);
static void dump_shdr(elfobj *elf, void *shdr, long shdr_cnt, char *name);
static void dump_dyn(elfobj *elf, void *dyn, long dyn_cnt);
#if 0
static void dump_sym(elfobj *elf, void *sym);
static void dump_rel(elfobj *elf, void *rel);
static void dump_rela(elfobj *elf, void *rela);
#endif
static void usage(int status);
static void parseargs(int argc, char *argv[]);

/* variables to control behavior */
static char be_verbose = 0;

/* misc dynamic tag caches */
static void *phdr_dynamic_void = NULL;

/* dump all internal elf info */
static void dumpelf(const char *filename, long file_cnt)
{
	elfobj *elf;
	unsigned long i, b;

	/* verify this is real ELF */
	if ((elf = readelf(filename)) == NULL)
		return;

	printf("#include <elf.h>\n");

	printf(
		"\n"
		"/*\n"
		" * ELF dump of '%s'\n"
		" *     %li (0x%lX) bytes\n"
		" */\n\n",
		filename, (unsigned long)elf->len, (unsigned long)elf->len);

	/* setup the struct to namespace this elf */
#define MAKE_STRUCT(B) \
	if (elf->elf_class == ELFCLASS ## B) { \
	Elf ## B ## _Ehdr *ehdr = EHDR ## B (elf->ehdr); \
	b = B; \
	printf( \
		"Elf%1$i_Dyn dumpedelf_dyn_%2$li[];\n" \
		"struct {\n" \
		"\tElf%1$i_Ehdr ehdr;\n" \
		"\tElf%1$i_Phdr phdrs[%3$li];\n" \
		"\tElf%1$i_Shdr shdrs[%4$li];\n" \
		"\tElf%1$i_Dyn *dyns;\n" \
		"} dumpedelf_%2$li = {\n\n", \
		B, file_cnt, \
		(long)EGET(ehdr->e_phnum), \
		(long)EGET(ehdr->e_shnum) \
	); \
	}
	MAKE_STRUCT(32)
	MAKE_STRUCT(64)

	/* dump the elf header */
	dump_ehdr(elf, elf->ehdr);

	/* dump the program headers */
	printf("\n.phdrs = {\n");
	if (elf->phdr) {
#define DUMP_PHDRS(B) \
		if (elf->elf_class == ELFCLASS ## B) { \
		Elf ## B ## _Ehdr *ehdr = EHDR ## B (elf->ehdr); \
		Elf ## B ## _Phdr *phdr = PHDR ## B (elf->phdr); \
		uint16_t phnum = EGET(ehdr->e_phnum); \
		for (i = 0; i < phnum; ++i, ++phdr) \
			dump_phdr(elf, phdr, i); \
		}
		DUMP_PHDRS(32)
		DUMP_PHDRS(64)
	} else {
		printf(" /* no program headers ! */ ");
	}
	printf("},\n");

	/* dump the section headers */
	printf("\n.shdrs = {\n");
	if (elf->shdr) {
#define DUMP_SHDRS(B) \
		if (elf->elf_class == ELFCLASS ## B) { \
		Elf ## B ## _Ehdr *ehdr = EHDR ## B (elf->ehdr); \
		Elf ## B ## _Shdr *shdr = SHDR ## B (elf->shdr); \
		uint16_t shstrndx = EGET(ehdr->e_shstrndx); \
		Elf ## B ## _Off offset = EGET(shdr[shstrndx].sh_offset); \
		uint16_t shnum = EGET(ehdr->e_shnum); \
		for (i = 0; i < shnum; ++i, ++shdr) \
			dump_shdr(elf, shdr, i, elf->vdata + offset + EGET(shdr->sh_name)); \
		}
		DUMP_SHDRS(32)
		DUMP_SHDRS(64)
	} else {
		printf(" /* no section headers ! */ ");
	}
	printf("},\n");

	/* finish the namespace struct and start the abitrary ones */
	printf("\n.dyns = dumpedelf_dyn_%li,\n", file_cnt);
	printf("};\n");

	/* start the arbitrary structs */
	printf("Elf%lu_Dyn dumpedelf_dyn_%li[] = {\n", b, file_cnt);
	if (phdr_dynamic_void) {
#define DUMP_DYNS(B) \
		if (elf->elf_class == ELFCLASS ## B) { \
		Elf ## B ## _Phdr *phdr = phdr_dynamic_void; \
		Elf ## B ## _Dyn *dyn = elf->vdata + EGET(phdr->p_offset); \
		i = 0; \
		do { \
			dump_dyn(elf, dyn++, i++); \
		} while (EGET(dyn->d_tag) != DT_NULL); \
		}
		DUMP_DYNS(32)
		DUMP_DYNS(64)
	} else {
		printf(" /* no dynamic tags ! */ ");
	}
	printf("};\n");

	/* get out of here */
	unreadelf(elf);
}
static void dump_ehdr(elfobj *elf, void *ehdr_void)
{
#define DUMP_EHDR(B) \
	if (elf->elf_class == ELFCLASS ## B) { \
	Elf ## B ## _Ehdr *ehdr = EHDR ## B (ehdr_void); \
	printf(".ehdr = {\n"); \
	printf("\t.e_ident = { /* (EI_NIDENT bytes) */\n" \
	       "\t\t/* [%i] EI_MAG:        */ 0x%X,'%c','%c','%c',\n" \
	       "\t\t/* [%i] EI_CLASS:      */ %i , /* (%s) */\n" \
	       "\t\t/* [%i] EI_DATA:       */ %i , /* (%s) */\n" \
	       "\t\t/* [%i] EI_VERSION:    */ %i , /* (%s) */\n" \
	       "\t\t/* [%i] EI_OSABI:      */ %i , /* (%s) */\n" \
	       "\t\t/* [%i] EI_ABIVERSION: */ %i ,\n" \
	       "\t\t/* [%i] EI_PAD:        */ 0x%02X /* x %i bytes */\n" \
	       /* "\t\t/ [%i] EI_BRAND:      / 0x%02X\n" */ \
	       "\t},\n", \
	       EI_MAG0, (unsigned int)ehdr->e_ident[EI_MAG0], ehdr->e_ident[EI_MAG1], ehdr->e_ident[EI_MAG2], ehdr->e_ident[EI_MAG3], \
	       EI_CLASS, (int)ehdr->e_ident[EI_CLASS], get_elfeitype(EI_CLASS, ehdr->e_ident[EI_CLASS]), \
	       EI_DATA, (int)ehdr->e_ident[EI_DATA], get_elfeitype(EI_DATA, ehdr->e_ident[EI_DATA]), \
	       EI_VERSION, (int)ehdr->e_ident[EI_VERSION], get_elfeitype(EI_VERSION, ehdr->e_ident[EI_VERSION]), \
	       EI_OSABI, (int)ehdr->e_ident[EI_OSABI], get_elfeitype(EI_OSABI, ehdr->e_ident[EI_OSABI]), \
	       EI_ABIVERSION, (int)ehdr->e_ident[EI_ABIVERSION], \
	       EI_PAD, (unsigned int)ehdr->e_ident[EI_PAD], EI_NIDENT - EI_PAD \
	       /* EI_BRAND, ehdr->e_ident[EI_BRAND] */ \
	); \
	printf("\t.e_type      = %-10i , /* (%s) */\n", (int)EGET(ehdr->e_type), get_elfetype(elf)); \
	printf("\t.e_machine   = %-10i , /* (%s) */\n", (int)EGET(ehdr->e_machine), get_elfemtype(elf)); \
	printf("\t.e_version   = %-10i ,\n", (int)EGET(ehdr->e_version)); \
	printf("\t.e_entry     = 0x%-8lX ,\n", (unsigned long)EGET(ehdr->e_entry)); \
	printf("\t.e_phoff     = %-10li , /* (bytes into file) */\n", (unsigned long)EGET(ehdr->e_phoff)); \
	printf("\t.e_shoff     = %-10li , /* (bytes into file) */\n", (unsigned long)EGET(ehdr->e_shoff)); \
	printf("\t.e_flags     = 0x%-8X ,\n", (unsigned int)EGET(ehdr->e_flags)); \
	printf("\t.e_ehsize    = %-10i , /* (bytes) */\n", (int)EGET(ehdr->e_ehsize)); \
	printf("\t.e_phentsize = %-10i , /* (bytes) */\n", (int)EGET(ehdr->e_phentsize)); \
	printf("\t.e_phnum     = %-10i , /* (program headers) */\n", (int)EGET(ehdr->e_phnum)); \
	printf("\t.e_shentsize = %-10i , /* (bytes) */\n", (int)EGET(ehdr->e_shentsize)); \
	printf("\t.e_shnum     = %-10i , /* (section headers) */\n", (int)EGET(ehdr->e_shnum)); \
	printf("\t.e_shstrndx  = %-10i\n", (int)EGET(ehdr->e_shstrndx)); \
	printf("},\n"); \
	}
	DUMP_EHDR(32)
	DUMP_EHDR(64)
}
static void dump_phdr(elfobj *elf, void *phdr_void, long phdr_cnt)
{
#define DUMP_PHDR(B) \
	if (elf->elf_class == ELFCLASS ## B) { \
	Elf ## B ## _Phdr *phdr = PHDR ## B (phdr_void); \
	switch (EGET(phdr->p_type)) { \
	case PT_DYNAMIC: phdr_dynamic_void = phdr_void; break; \
	} \
	printf("/* Program Header #%li 0x%lX */\n{\n", phdr_cnt, (unsigned long)phdr_void - (unsigned long)elf->data); \
	printf("\t.p_type   = %-10li , /* [%s] */\n", (long)EGET(phdr->p_type), get_elfptype(EGET(phdr->p_type))); \
	printf("\t.p_offset = %-10li ,\n", (long)EGET(phdr->p_offset)); \
	printf("\t.p_vaddr  = 0x%-8lX ,\n", (unsigned long)EGET(phdr->p_vaddr)); \
	printf("\t.p_paddr  = 0x%-8lX ,\n", (unsigned long)EGET(phdr->p_paddr)); \
	printf("\t.p_filesz = %-10li ,\n", (long)EGET(phdr->p_filesz)); \
	printf("\t.p_memsz  = %-10li ,\n", (long)EGET(phdr->p_memsz)); \
	printf("\t.p_flags  = %-10li ,\n", (long)EGET(phdr->p_flags)); \
	printf("\t.p_align  = %-10li\n", (long)EGET(phdr->p_align)); \
	printf("},\n"); \
	}
	DUMP_PHDR(32)
	DUMP_PHDR(64)
}
static void dump_shdr(elfobj *elf, void *shdr_void, long shdr_cnt, char *name)
{
	unsigned long i;
#define DUMP_SHDR(B) \
	if (elf->elf_class == ELFCLASS ## B) { \
	Elf ## B ## _Shdr *shdr = SHDR ## B (shdr_void); \
	uint32_t type = EGET(shdr->sh_type); \
	uint ## B ## _t size = EGET(shdr->sh_size); \
	printf("/* Section Header #%li '%s' 0x%lX */\n{\n", \
	       shdr_cnt, name, (unsigned long)shdr_void - (unsigned long)elf->data); \
	printf("\t.sh_name      = %-10i ,\n", (int)EGET(shdr->sh_name)); \
	printf("\t.sh_type      = %-10i , /* [%s] */\n", (int)EGET(shdr->sh_type), get_elfshttype(type)); \
	printf("\t.sh_flags     = %-10li ,\n", (long)EGET(shdr->sh_flags)); \
	printf("\t.sh_addr      = 0x%-8lX ,\n", (unsigned long)EGET(shdr->sh_addr)); \
	printf("\t.sh_offset    = %-10i , /* (bytes) */\n", (int)EGET(shdr->sh_offset)); \
	printf("\t.sh_size      = %-10li , /* (bytes) */\n", (long)EGET(shdr->sh_size)); \
	printf("\t.sh_link      = %-10i ,\n", (int)EGET(shdr->sh_link)); \
	printf("\t.sh_info      = %-10i ,\n", (int)EGET(shdr->sh_info)); \
	printf("\t.sh_addralign = %-10li ,\n", (long)EGET(shdr->sh_addralign)); \
	printf("\t.sh_entsize   = %-10li\n", (long)EGET(shdr->sh_entsize)); \
	if (size && be_verbose) { \
		void *vdata = elf->vdata + EGET(shdr->sh_offset); \
		unsigned char *data = vdata; \
		switch (type) { \
		case SHT_PROGBITS: { \
			if (strcmp(name, ".interp") == 0) { \
				printf("\n\t/* ELF interpreter: %s */\n", data); \
				break; \
			} \
			if (strcmp(name, ".comment") != 0) \
				break; \
		} \
		case SHT_STRTAB: { \
			char b; \
			printf("\n\t/%c section dump:\n", '*'); \
			b = 1; \
			if (type == SHT_PROGBITS) --data; \
			for (i = 0; i < size; ++i) { \
				++data; \
				if (*data) { \
					if (b) printf("\t * "); \
					printf("%c", *data); \
					b = 0; \
				} else if (!b) { \
					printf("\n"); \
					b = 1; \
				} \
			} \
			printf("\t */\n"); \
			break; \
		} \
		case SHT_DYNSYM: { \
			Elf##B##_Sym *sym = vdata; \
			printf("\n\t/%c section dump:\n", '*'); \
			for (i = 0; i < EGET(shdr->sh_size) / EGET(shdr->sh_entsize); ++i) { \
				printf("\t * Elf%i_Sym sym%li = {\n", B, (long)i); \
				printf("\t * \t.st_name  = %i,\n", (unsigned int)EGET(sym->st_name)); \
				printf("\t * \t.st_value = 0x%lX,\n", (unsigned long)EGET(sym->st_value)); \
				printf("\t * \t.st_size  = %li, (bytes)\n", (unsigned long)EGET(sym->st_size)); \
				printf("\t * \t.st_info  = %i,\n", (unsigned int)EGET(sym->st_info)); \
				printf("\t * \t.st_other = %i,\n", (unsigned int)EGET(sym->st_other)); \
				printf("\t * \t.st_shndx = %li\n", (unsigned long)EGET(sym->st_shndx)); \
				printf("\t * };\n"); \
				++sym; \
			} \
			printf("\t */\n"); \
			break; \
		} \
		default: { \
			if (be_verbose <= 1) \
				break; \
			printf("\n\t/%c section dump:\n", '*'); \
			for (i = 0; i < size; ++i) { \
				++data; \
				if (i % 20 == 0) printf("\t * "); \
				printf("%.2X ", *data); /* this line can cause segfaults */ \
				if (i % 20 == 19) printf("\n"); \
			} \
			if (i % 20) printf("\n"); \
			printf("\t */\n"); \
		} \
		} \
	} \
	printf("},\n"); \
	}
	DUMP_SHDR(32)
	DUMP_SHDR(64)
}
static void dump_dyn(elfobj *elf, void *dyn_void, long dyn_cnt)
{
#define DUMP_DYN(B) \
	if (elf->elf_class == ELFCLASS ## B) { \
	Elf ## B ## _Dyn *dyn = dyn_void; \
	unsigned long tag = EGET(dyn->d_tag); \
	printf("/* Dynamic tag #%li '%s' 0x%lX */\n{\n", \
	       dyn_cnt, get_elfdtype(tag), (unsigned long)dyn_void - (unsigned long)elf->data); \
	printf("\t.d_tag     = 0x%-8lX ,\n", tag); \
	printf("\t.d_un      = {\n"); \
	printf("\t\t.d_val = 0x%-8lX ,\n", (unsigned long)EGET(dyn->d_un.d_val)); \
	printf("\t\t.d_ptr = 0x%-8lX ,\n", (unsigned long)EGET(dyn->d_un.d_val)); \
	printf("\t},\n"); \
	printf("},\n"); \
	}
	DUMP_DYN(32)
	DUMP_DYN(64)
}

/* usage / invocation handling functions */
#define PARSE_FLAGS "vhV"
#define a_argument required_argument
static struct option const long_opts[] = {
	{"verbose",   no_argument, NULL, 'v'},
	{"help",      no_argument, NULL, 'h'},
	{"version",   no_argument, NULL, 'V'},
	{NULL,        no_argument, NULL, 0x0}
};
static const char * const opts_help[] = {
	"Be verbose (can be specified more than once)",
	"Print this help and exit",
	"Print version and exit",
	NULL
};

/* display usage and exit */
static void usage(int status)
{
	int i;
	printf("* Dump internal ELF structure\n\n"
	       "Usage: %s <file1> [file2 fileN ...]\n\n", argv0);
	printf("Options:\n");
	for (i = 0; long_opts[i].name; ++i)
		if (long_opts[i].has_arg == no_argument)
			printf("  -%c, --%-13s* %s\n", long_opts[i].val,
			       long_opts[i].name, opts_help[i]);
		else
			printf("  -%c, --%-6s <arg> * %s\n", long_opts[i].val,
			       long_opts[i].name, opts_help[i]);
	exit(status);
}

/* parse command line arguments and preform needed actions */
static void parseargs(int argc, char *argv[])
{
	int flag;

	opterr = 0;
	while ((flag=getopt_long(argc, argv, PARSE_FLAGS, long_opts, NULL)) != -1) {
		switch (flag) {

		case 'V':                        /* version info */
			printf("pax-utils-%s: %s compiled %s\n%s\n"
			       "%s written for Gentoo by <solar and vapier @ gentoo.org>\n",
			       VERSION, __FILE__, __DATE__, rcsid, argv0);
			exit(EXIT_SUCCESS);
			break;
		case 'h': usage(EXIT_SUCCESS); break;

		case 'v': be_verbose = (be_verbose % 20) + 1; break;

		case ':':
			err("Option missing parameter");
		case '?':
			err("Unknown option");
		default:
			err("Unhandled option '%c'", flag);
		}
	}

	if (optind == argc)
		err("Nothing to dump !?");

	{
	long file_cnt = 0;

	while (optind < argc)
		dumpelf(argv[optind++], file_cnt++);
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2)
		usage(EXIT_FAILURE);
	parseargs(argc, argv);
	return EXIT_SUCCESS;
}
