/* This file was downloaded 2009/02/02;13:56EST from
 * http://www.tachyonsoft.com/downloads/elfdump_linux.tar.gz
 * (also referenced from http://www.tachyonsoft.com/elf.html)
 * JCA */
/**************************** elfdump.c *******************************/
/*****                                                            *****/
/***** This program was originally written by David Bond.  It has *****/
/***** been placed into the public domain and thus may be freely  *****/
/***** modified and redistributed with no restrictions whatsoever.*****/
/*****                                                            *****/
/*****   THERE IS NO WARRENTY FOR THIS PROGRAM, EITHER EXPRESS    *****/
/*****   OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED   *****/
/*****   WARRENTIES OF MERCHANTABILITY AND FITNESS FOR A          *****/
/*****   PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY   *****/
/*****   AND PERFORMANCE OF THS PROGRAM IS WITH YOU.  SHOULD THE  *****/
/*****   PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL      *****/
/*****   NECESSARY SERVICING, REPAIR OR CORRECTION.               *****/
/*****                                                            *****/
/**********************************************************************/

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define I64 __int64
#else
#define I64 long long
#endif
#define I32 int
#define I16 short
typedef unsigned char U8;

typedef unsigned I32 Elf32_Addr;
typedef unsigned I32 Elf32_Off;
typedef          I32 Elf32_Sword;
typedef unsigned I32 Elf32_Word;
typedef unsigned I16 Elf32_Half;

typedef unsigned I64 Elf64_Addr;
typedef unsigned I64 Elf64_Off;
typedef unsigned I64 Elf64_Xword;
typedef          I64 Elf64_Sxword;
typedef          I32 Elf64_Sword;
typedef unsigned I32 Elf64_Word;
typedef unsigned I16 Elf64_Half;

typedef struct {
#define EI_NIDENT (16)
    U8          e_ident[EI_NIDENT];
#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_OSABI 7
#define EI_ABIVERSION 8
#define ELFMAG0 ((U8)0x7F)
#define ELFMAG1 ((U8)'E')
#define ELFMAG2 ((U8)'L')
#define ELFMAG3 ((U8)'F')
#define ELFCLASS32 ((U8)1)
#define ELFCLASS64 ((U8)2)
#define ELFDATALSB ((U8)1)
#define ELFDATAMSB ((U8)2)
#define ELFOSABI_LINUX ((U8)3)
    Elf32_Half  e_type;
#define ET_REL  ((Elf32_Half)1)
#define ET_EXEC ((Elf32_Half)2)
#define ET_DYN  ((Elf32_Half)3)
#define ET_CORE ((Elf32_Half)4)
    Elf32_Half  e_machine;
#define EM_386  ((Elf32_Half)0x0003)
#define EM_S390 ((Elf32_Half)0x0016)
#define EM_A390 ((Elf32_Half)0xA390)
    Elf32_Word  e_version;
#define EV_CURRENT 1
    Elf32_Addr  e_entry;
    Elf32_Off   e_phoff;
    Elf32_Off   e_shoff;
    Elf32_Word  e_flags;
    Elf32_Half  e_ehsize;
    Elf32_Half  e_phentsize;
    Elf32_Half  e_phnum;
    Elf32_Half  e_shentsize;
    Elf32_Half  e_shnum;
    Elf32_Half  e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    U8          e_ident[EI_NIDENT];
    Elf64_Half  e_type;
    Elf64_Half  e_machine;
    Elf64_Word  e_version;
    Elf64_Addr  e_entry;
    Elf64_Off   e_phoff;
    Elf64_Off   e_shoff;
    Elf64_Word  e_flags;
    Elf64_Half  e_ehsize;
    Elf64_Half  e_phentsize;
    Elf64_Half  e_phnum;
    Elf64_Half  e_shentsize;
    Elf64_Half  e_shnum;
    Elf64_Half  e_shstrndx;
} Elf64_Ehdr;

#define SHN_UNDEF  (0)
#define SHN_ABS    (0xFFF1)
#define SHN_COMMON (0xFFF2)
typedef struct {
    Elf32_Word  sh_name;
    Elf32_Word  sh_type;
#define SHT_NULL     (0)
#define SHT_PROGBITS (1)
#define SHT_SYMTAB   (2)
#define SHT_STRTAB   (3)
#define SHT_RELA     (4)
#define SHT_HASH     (5)
#define SHT_DYNAMIC  (6)
#define SHT_NOTE     (7)
#define SHT_NOBITS   (8)
#define SHT_REL      (9)
#define SHT_SHLIB    (10)
#define SHT_DYNSYM   (11)
#define SHT_INIT_ARRAY    (14)
#define SHT_FINI_ARRAY    (15)
#define SHT_PREINIT_ARRAY (16)
#define SHT_GROUP         (17)
#define SHT_SYMTAB_SHNDX  (18)
#define SHT_GNU_verdef  (0x6FFFFFFD)
#define SHT_GNU_verneed (0x6FFFFFFE)
#define SHT_GNU_versym  (0x6FFFFFFF)
    Elf32_Word  sh_flags;
#define SHF_WRITE      (1)
#define SHF_ALLOC      (2)
#define SHF_EXECINSTR  (4)
#define SHF_MERGE      (16)
#define SHF_STRINGS    (32)
#define SHF_INFO_LINK  (64)
#define SHF_LINK_ORDER (128)
    Elf32_Addr  sh_addr;
    Elf32_Off   sh_offset;
    Elf32_Word  sh_size;
    Elf32_Word  sh_link;
    Elf32_Word  sh_info;
    Elf32_Word  sh_addralign;
    Elf32_Word  sh_entsize;
} Elf32_Shdr;

typedef struct {
    Elf64_Word  sh_name;
    Elf64_Word  sh_type;
    Elf64_Xword sh_flags;
    Elf64_Addr  sh_addr;
    Elf64_Off   sh_offset;
    Elf64_Xword sh_size;
    Elf64_Word  sh_link;
    Elf64_Word  sh_info;
    Elf64_Xword sh_addralign;
    Elf64_Xword sh_entsize;
} Elf64_Shdr;

typedef struct {
    Elf32_Word  p_type;
#define PT_NULL    (0)
#define PT_LOAD    (1)
#define PT_DYNAMIC (2)
#define PT_INTERP  (3)
#define PT_NOTE    (4)
#define PT_SHLIB   (5)
#define PT_PHDR    (6)
    Elf32_Off   p_offset;
    Elf32_Addr  p_vaddr;
    Elf32_Addr  p_paddr;
    Elf32_Word  p_filesz;
    Elf32_Word  p_memsz;
    Elf32_Word  p_flags;
#define PF_X (1)
#define PF_W (2)
#define PF_R (4)
    Elf32_Word  p_align;
} Elf32_Phdr;

typedef struct {
    Elf64_Word  p_type;
    Elf64_Word  p_flags;
    Elf64_Off   p_offset;
    Elf64_Addr  p_vaddr;
    Elf64_Addr  p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
} Elf64_Phdr;

#define STN_UNDEF (0)
typedef struct {
    Elf32_Word  st_name;
    Elf32_Addr  st_value;
    Elf32_Word  st_size;
    U8          st_info;
#define ELF_ST_BIND(i) ((i)>>4)
#define ELF_ST_TYPE(i) ((i)&0x0f)
#define ELF_ST_INFO(b,t) (((b)<<4)|((t)&0x0f))
#define STB_LOCAL   (0)
#define STB_GLOBAL  (1)
#define STB_WEAK    (2)
#define STT_NOTYPE  (0)
#define STT_OBJECT  (1)
#define STT_FUNC    (2)
#define STT_SECTION (3)
#define STT_FILE    (4)
#define STT_COMMON  (5)
    U8          st_other;
#define STV_INTERNAL  (1)
#define STV_HIDDEN    (2)
#define STV_PROTECTED (3)
    Elf32_Half  st_shndx;
} Elf32_Sym;

typedef struct {
    Elf64_Word  st_name;
    U8          st_info;
    U8          st_other;
    Elf64_Half  st_shndx;
    Elf64_Addr  st_value;
    Elf64_Xword st_size;
} Elf64_Sym;

typedef struct {
    Elf32_Addr  r_offset;
    Elf32_Word  r_info;
#define ELF32_R_SYM(i) ((i)>>8)
#define ELF32_R_TYPE(i) ((U8)(i & 0xFF))
#define ELF32_R_INFO(s,t) (((s)<<8)|(U8)(t))
} Elf32_Rel;

typedef struct {
    Elf64_Addr  r_offset;
    Elf64_Xword r_info;
#define ELF64_R_SYM(i) ((Elf64_Word)((i)>>32))
#define ELF64_R_TYPE(i) ((Elf64_Word)(i & 0xFFFFFFFF))
#define ELF64_R_INFO(s,t) ((((Elf64_Xword)(s))<<32)|(Elf64_Word)(t))
} Elf64_Rel;

typedef struct {
    Elf32_Addr  r_offset;
    Elf32_Word  r_info;
    Elf32_Sword r_addend;
} Elf32_Rela;

typedef struct {
    Elf64_Addr   r_offset;
    Elf64_Xword  r_info;
    Elf64_Sxword r_addend;
} Elf64_Rela;

typedef struct {
    Elf32_Word  n_key_size;
#define NI_NOTE (4)
    Elf32_Word  n_data_size;
    Elf32_Word  n_type;
#define NT_PRSTATUS (1)
#define NT_FPREGSET (2)
#define NT_PRPSINFO (3)
#define NT_TASKSTRUCT (4)
} Elf32_Note;

#undef BYTE_ORDER
#if defined(_WIN32) || defined(__i386)
#define BYTE_ORDER ELFDATALSB
#else
#define BYTE_ORDER ELFDATAMSB
#endif

typedef struct {
    const char * pszPathName;
    FILE       * pFile;
    Elf32_Ehdr   Ehdr;
    Elf32_Shdr * paShdr;
    Elf32_Phdr * paPhdr;
    void     * * papSection;
    void     * * papSegment;
} ElfFile;

static void swap2(
    U8 * hword)
{
    U8 t;
    t        = hword[0];
    hword[0] = hword[1];
    hword[1] = t;
} /* swap2 */

static void swap4(
    U8 * word)
{
    U8 t;
    t       = word[0];
    word[0] = word[3];
    word[3] = t;
    t       = word[1];
    word[1] = word[2];
    word[2] = t;
} /* swap4 */

static void swap8(
    U8 * xword)
{
    U8 t;
    t        = xword[0];
    xword[0] = xword[7];
    xword[7] = t;
    t        = xword[1];
    xword[1] = xword[6];
    xword[6] = t;
    t        = xword[2];
    xword[2] = xword[5];
    xword[5] = t;
    t        = xword[3];
    xword[3] = xword[4];
    xword[4] = t;
} /* swap8 */
#define SWAP2(x) swap2((U8 *)x)
#define SWAP4(x) swap4((U8 *)x)
#define SWAP8(x) swap8((U8 *)x)

static void * safe_malloc(
    size_t size)
{
    void * ret = malloc(size);
    if (ret == NULL) {
        fprintf(stderr, "Heap storage exhausted, failed to allocate %lu bytes.\n",
                size);
        exit(1);
    }
    return ret;
} /* safe_malloc */

static void * safe_calloc(
    size_t size,
    size_t count)
{
    void * ret = calloc(size, count);
    if (ret == NULL) {
        fprintf(stderr, "Heap storage exhausted, failed to allocate %lu bytes.\n",
                size * count);
        exit(1);
    }
    return ret;
} /* safe_calloc */

static int iSeekElfFile(
    ElfFile * pElfFile,
    Elf32_Off offset)
{
    if (fseek(pElfFile->pFile, offset, SEEK_SET) != 0) {
        fprintf(stderr, "Error seeking \"%s\" to offset %lu: %s\n",
                pElfFile->pszPathName,
                offset,
                strerror(errno));
        return 1;
    }
    return 0;
} /* iSeekElfFile */

static const char * pszReadElfFile(
    void    * buffer,
    size_t    size,
    size_t    count,
    ElfFile * pElfFile)
{
    if (fread(buffer, size, count, pElfFile->pFile) != count) {
        if (feof(pElfFile->pFile))
            return "Invalid ELF file";
        return strerror(errno);
    }
    return NULL;
} /* pszReadElfFile */

static int iLoadElfFile(
    ElfFile * pElfFile)
{
    const char * pszMsg = NULL;

    pElfFile->pFile = fopen(pElfFile->pszPathName, "rb");
    if (pElfFile->pFile == NULL) {
        fprintf(stderr, "Error opening \"%s\": %s\n",
                pElfFile->pszPathName, strerror(errno));
        return 1;
    }
    pszMsg = pszReadElfFile(&(pElfFile->Ehdr),
                            sizeof(pElfFile->Ehdr),
                            1,
                            pElfFile);
    if (pszMsg == NULL) {
        if ((pElfFile->Ehdr.e_ident[EI_MAG0] != ELFMAG0) ||
            (pElfFile->Ehdr.e_ident[EI_MAG1] != ELFMAG1) ||
            (pElfFile->Ehdr.e_ident[EI_MAG2] != ELFMAG2) ||
            (pElfFile->Ehdr.e_ident[EI_MAG3] != ELFMAG3) ||
            (pElfFile->Ehdr.e_ident[EI_VERSION] != EV_CURRENT)) {
            pszMsg = "Invalid ELF file";
        }
    }
    if (pszMsg == NULL) {
        if (pElfFile->Ehdr.e_ident[EI_DATA] != BYTE_ORDER) {
            SWAP2(&(pElfFile->Ehdr.e_type));
            SWAP2(&(pElfFile->Ehdr.e_machine));
            SWAP4(&(pElfFile->Ehdr.e_version));
        }
        switch(pElfFile->Ehdr.e_type) {
          case ET_REL:
          case ET_EXEC:
          case ET_DYN:
          case ET_CORE:
            break;
          default:
            pszMsg = "Invalid ELF file type";
        }
    }
    if (pszMsg == NULL) {
        switch (pElfFile->Ehdr.e_ident[EI_CLASS]) {
          case ELFCLASS32:
            switch(pElfFile->Ehdr.e_machine) {
              case EM_386:
                if (pElfFile->Ehdr.e_ident[EI_DATA] != ELFDATALSB)
                    pszMsg = "Invalid Intel/x86 ELF file";
                break;
              case EM_S390:
              case EM_A390:
                if (pElfFile->Ehdr.e_ident[EI_DATA] == ELFDATAMSB)
                    break;
              default:
                pszMsg = "Invalid S/390 32-bit ELF file";
            }
            break;
          case ELFCLASS64:
            if ((pElfFile->Ehdr.e_machine != EM_S390) ||
                (pElfFile->Ehdr.e_ident[EI_DATA] != ELFDATAMSB))
                pszMsg = "Invalid S/390 64-bit ELF file";
            else pszMsg = "64-bit ELF file is not supported";
            break;
          default:
            pszMsg = "Invalid 32-bit ELF file";
        }
    }

    if ((pszMsg == NULL) &&
        (pElfFile->Ehdr.e_ident[EI_DATA] != BYTE_ORDER)) {
        SWAP4(&(pElfFile->Ehdr.e_entry));
        SWAP4(&(pElfFile->Ehdr.e_phoff));
        SWAP4(&(pElfFile->Ehdr.e_shoff));
        SWAP4(&(pElfFile->Ehdr.e_flags));
        SWAP2(&(pElfFile->Ehdr.e_ehsize));
        SWAP2(&(pElfFile->Ehdr.e_phentsize));
        SWAP2(&(pElfFile->Ehdr.e_phnum));
        SWAP2(&(pElfFile->Ehdr.e_shentsize));
        SWAP2(&(pElfFile->Ehdr.e_shnum));
        SWAP2(&(pElfFile->Ehdr.e_shstrndx));
    }

    if ((pszMsg == NULL) && (pElfFile->Ehdr.e_shnum != 0)) {
        if (iSeekElfFile(pElfFile, pElfFile->Ehdr.e_shoff))
            return 1;
        pElfFile->paShdr     = safe_calloc(pElfFile->Ehdr.e_shentsize,
                                           pElfFile->Ehdr.e_shnum);
        pElfFile->papSection = safe_calloc(sizeof(void *),
                                           pElfFile->Ehdr.e_shnum);
        pszMsg = pszReadElfFile(pElfFile->paShdr,
                                pElfFile->Ehdr.e_shentsize,
                                pElfFile->Ehdr.e_shnum,
                                pElfFile);
    }
    if (pszMsg == NULL) {
        unsigned shnum;
        for (shnum = 0; shnum < pElfFile->Ehdr.e_shnum; ++shnum) {
            if (pElfFile->Ehdr.e_ident[EI_DATA] != BYTE_ORDER) {
                SWAP4(&(pElfFile->paShdr[shnum].sh_name));
                SWAP4(&(pElfFile->paShdr[shnum].sh_type));
                SWAP4(&(pElfFile->paShdr[shnum].sh_flags));
                SWAP4(&(pElfFile->paShdr[shnum].sh_addr));
                SWAP4(&(pElfFile->paShdr[shnum].sh_offset));
                SWAP4(&(pElfFile->paShdr[shnum].sh_size));
                SWAP4(&(pElfFile->paShdr[shnum].sh_link));
                SWAP4(&(pElfFile->paShdr[shnum].sh_info));
                SWAP4(&(pElfFile->paShdr[shnum].sh_addralign));
                SWAP4(&(pElfFile->paShdr[shnum].sh_entsize));
            }
            if (pElfFile->paShdr[shnum].sh_size == 0)
                continue;
            switch (pElfFile->paShdr[shnum].sh_type) {
              case SHT_NULL:
              case SHT_NOBITS:
                continue;
            }
            pElfFile->papSection[shnum] =
                safe_malloc(pElfFile->paShdr[shnum].sh_size);
            if (iSeekElfFile(pElfFile, pElfFile->paShdr[shnum].sh_offset))
                return 1;
            pszMsg = pszReadElfFile(pElfFile->papSection[shnum],
                                    pElfFile->paShdr[shnum].sh_size,
                                    1,
                                    pElfFile);
            if (pszMsg != NULL)
                break;
            if (pElfFile->Ehdr.e_ident[EI_DATA] != BYTE_ORDER) {
                switch (pElfFile->paShdr[shnum].sh_type) {
                  case SHT_SYMTAB:
                  case SHT_DYNSYM:
                    {
                        Elf32_Sym * pSym;
                        unsigned    count;
                        pSym  = pElfFile->papSection[shnum];
                        count = pElfFile->paShdr[shnum].sh_size /
                                pElfFile->paShdr[shnum].sh_entsize;
                        while (count != 0) {
                            SWAP4(&(pSym->st_name));
                            SWAP4(&(pSym->st_value));
                            SWAP4(&(pSym->st_size));
                            SWAP2(&(pSym->st_shndx));
                            ++pSym;
                            --count;
                        }
                    }
                    break;
                  case SHT_REL:
                    {
                        Elf32_Rel * pRel;
                        unsigned    count;
                        pRel  = pElfFile->papSection[shnum];
                        count = pElfFile->paShdr[shnum].sh_size /
                                pElfFile->paShdr[shnum].sh_entsize;
                        while (count != 0) {
                            SWAP4(&(pRel->r_offset));
                            SWAP4(&(pRel->r_info));
                            ++pRel;
                            --count;
                        }
                    }
                    break;
                  case SHT_RELA:
                    {
                        Elf32_Rela * pRela;
                        unsigned     count;
                        pRela = pElfFile->papSection[shnum];
                        count = pElfFile->paShdr[shnum].sh_size /
                                pElfFile->paShdr[shnum].sh_entsize;
                        while (count != 0) {
                            SWAP4(&(pRela->r_offset));
                            SWAP4(&(pRela->r_info));
                            SWAP4(&(pRela->r_addend));
                            ++pRela;
                            --count;
                        }
                    }
                    break;
                }
            }
        }
    }
    if ((pszMsg == NULL) && (pElfFile->Ehdr.e_phnum != 0)) {
        if (iSeekElfFile(pElfFile, pElfFile->Ehdr.e_phoff))
            return 1;
        pElfFile->paPhdr = safe_calloc(pElfFile->Ehdr.e_phentsize,
                                       pElfFile->Ehdr.e_phnum);
        if (pElfFile->Ehdr.e_type == ET_CORE)
            pElfFile->papSegment = safe_calloc(sizeof(void *),
                                               pElfFile->Ehdr.e_phnum);
        pszMsg = pszReadElfFile(pElfFile->paPhdr,
                                pElfFile->Ehdr.e_phentsize,
                                pElfFile->Ehdr.e_phnum,
                                pElfFile);
    }
    if (pszMsg == NULL) {
        unsigned phnum;
        for (phnum = 0; phnum < pElfFile->Ehdr.e_phnum; ++phnum) {
            if (pElfFile->Ehdr.e_ident[EI_DATA] != BYTE_ORDER) {
                SWAP4(&(pElfFile->paPhdr[phnum].p_type));
                SWAP4(&(pElfFile->paPhdr[phnum].p_offset));
                SWAP4(&(pElfFile->paPhdr[phnum].p_vaddr));
                SWAP4(&(pElfFile->paPhdr[phnum].p_paddr));
                SWAP4(&(pElfFile->paPhdr[phnum].p_filesz));
                SWAP4(&(pElfFile->paPhdr[phnum].p_memsz));
                SWAP4(&(pElfFile->paPhdr[phnum].p_flags));
                SWAP4(&(pElfFile->paPhdr[phnum].p_align));
            }
            if ((pElfFile->Ehdr.e_type == ET_CORE) &&
                (pElfFile->paPhdr[phnum].p_filesz != 0)) {
                pElfFile->papSegment[phnum] =
                    safe_malloc(pElfFile->paPhdr[phnum].p_filesz);
                if (iSeekElfFile(pElfFile, pElfFile->paPhdr[phnum].p_offset))
                    return 1;
                pszMsg = pszReadElfFile(pElfFile->papSegment[phnum],
                                        pElfFile->paPhdr[phnum].p_filesz,
                                        1,
                                        pElfFile);
                if (pszMsg != NULL)
                    break;
            }
        }
    }

    if (pszMsg != NULL) {
        fprintf(stderr, "Error reading \"%s\": %s\n",
                    pElfFile->pszPathName, pszMsg);
        return 1;
    }
    return 0;
} /* iLoadElfFile */

static void vDumpProgbits(
    const U8 * pBytes,
    unsigned   offset,
    unsigned   size)
{
    unsigned bytes;
    unsigned pos;
    char szLine[67];

    szLine[64] = '|';
    szLine[65] = '\n';
    szLine[66] = '\0';
    while (size != 0) {
        bytes = 16;
        if (bytes > size)
            bytes = size;
        sprintf(szLine, "%.8X", offset);
        memset(szLine+8, ' ', 56);
        szLine[47] = '|';
        for (pos = 0; pos < bytes; ++pos) {
            sprintf(szLine+10+(pos*2)+(pos/4) ,"%.2X", pBytes[pos]);
            szLine[12+(pos*2)+(pos/4)] = ' ';
            if ((pBytes[pos] > ' ') && (pBytes[pos] < '\x7F'))
                szLine[48+pos] = pBytes[pos];
        }
        fputs(szLine, stdout);
        offset += bytes;
        pBytes += bytes;
        size   -= bytes;
    }
} /* vDumpProgbits */

/* For Intel/x86 */
static void vDumpRel(
    const Elf32_Rel  * paRel,
    unsigned           count,
    const Elf32_Sym  * paSym,
    const char       * pacStrtab,
    const Elf32_Shdr * paShdr,
    const char       * pacShStrtab)
{
    unsigned rel;
    unsigned type;
    unsigned sym;
    static const char aszTypes[][12] = {
        "NONE",         /* 0 */
        "32",           /* 1 */
        "PC32",         /* 2 */
        "GOT32",        /* 3 */
        "PLT32",        /* 4 */
        "COPY",         /* 5 */
        "GLOB_DAT",     /* 6 */
        "JMP_SLOT",     /* 7 */
        "RELATIVE",     /* 8 */
        "GOTOFF",       /* 9 */
        "GOTPC"         /* 10 */
        };

    for (rel = 0; rel < count; ++rel) {
        fprintf(stdout, "%4u offset=%.8X type=",
                rel, paRel[rel].r_offset);
        type = ELF32_R_TYPE(paRel[rel].r_info);
        if (type <= 10)
            fputs(aszTypes[type], stdout);
        else fprintf(stdout, "0x%X", type);
        sym = ELF32_R_SYM(paRel[rel].r_info);
        if (ELF_ST_TYPE(paSym[sym].st_info) == STT_SECTION) {
            fprintf(stdout, " section=%s\n",
                    pacShStrtab + paShdr[paSym[sym].st_shndx].sh_name);
        } else {
            fprintf(stdout, " sym=\"%s\"\n",
                    pacStrtab + paSym[sym].st_name);
        }
    }
} /* vDumpRel */

/* For S/390 */
static void vDumpRela(
    const Elf32_Rela * paRela,
    unsigned           count,
    const Elf32_Sym  * paSym,
    const char       * pacStrtab,
    const Elf32_Shdr * paShdr,
    const char       * pacShStrtab)
{
    unsigned rel;
    unsigned type;
    unsigned sym;
    static const char aszTypes[][12] = {
        "NONE",         /* 0 */
        "8",            /* 1 */
        "12",           /* 2 */
        "16",           /* 3 */
        "32",           /* 4 */
        "PC32",         /* 5 */
        "GOT12",        /* 6 */
        "GOT32",        /* 7 */
        "PLT32",        /* 8 */
        "COPY",         /* 9 */
        "GLOB_DAT",     /* 10 */
        "JMP_SLOT",     /* 11 */
        "RELATIVE",     /* 12 */
        "GOTOFF",       /* 13 */
        "GOTPC",        /* 14 */
        "GOT16",        /* 15 */
        "PC16",         /* 16 */
        "PC16DBL",      /* 17 */
        "PLT16DBL",     /* 18 */
        "PC32DBL",      /* 19 */
        "PLT32DBL",     /* 20 */
        "GOTPCDBL",     /* 21 */
        "64",           /* 22 */
        "PC64",         /* 23 */
        "GOT64",        /* 24 */
        "PLT64",        /* 25 */
        "GOTENT"        /* 26 */
        };

    for (rel = 0; rel < count; ++rel) {
        fprintf(stdout, "%4u offset=%.8X addend=%.8X type=",
                rel, paRela[rel].r_offset, paRela[rel].r_addend);
        type = ELF32_R_TYPE(paRela[rel].r_info);
        if (type <= 26)
            fputs(aszTypes[type], stdout);
        else fprintf(stdout, "0x%X", type);
        sym = ELF32_R_SYM(paRela[rel].r_info);
        if (ELF_ST_TYPE(paSym[sym].st_info) == STT_SECTION) {
            fprintf(stdout, " section=%s\n",
                    pacShStrtab + paShdr[paSym[sym].st_shndx].sh_name);
        } else {
            fprintf(stdout, " sym=\"%s\"\n",
                    pacStrtab + paSym[sym].st_name);
        }
    }
} /* vDumpRela */

static void vDumpSymtab(
    Elf32_Half         e_type,
    const Elf32_Sym  * paSym,
    unsigned           count,
    const char       * pacStrtab,
    const Elf32_Shdr * paShdr,
    const char       * pacShStrtab)
{
    unsigned sym;

    for (sym = 1; sym < count; ++sym) {
        fprintf(stdout, "%4u  ", sym);
        switch (ELF_ST_TYPE(paSym[sym].st_info)) {
          case STT_SECTION:
            fprintf(stdout, ".section %s\n",
                    pacShStrtab + paShdr[paSym[sym].st_shndx].sh_name);
            continue;
          case STT_FILE:
            fprintf(stdout, ".file \"%s\"\n",
                    pacStrtab + paSym[sym].st_name);
            continue;
        }
        switch (ELF_ST_BIND(paSym[sym].st_info)) {
          case STB_LOCAL:
            fputs(".local ", stdout);
            break;
          case STB_GLOBAL:
            fputs(".globl ", stdout);
            break;
          case STB_WEAK:
            fputs(".weak ", stdout);
        }
        fputs(pacStrtab + paSym[sym].st_name, stdout);
        switch (ELF_ST_TYPE(paSym[sym].st_info)) {
          case STT_OBJECT:
            fputs(",@OBJECT", stdout);
            break;
          case STT_FUNC:
            fputs(",@FUNCTION", stdout);
            break;
        }
        switch (paSym[sym].st_shndx) {
          case SHN_UNDEF:
            fputs(",.extrn", stdout);
            break;
          case SHN_ABS:
            fprintf(stdout, ",VALUE=0x%.2X",
                    paSym[sym].st_value);
            break;
          case SHN_COMMON:
            fprintf(stdout, ",COMMON,ALIGN=%u",
                    paSym[sym].st_value);
            break;
          default:
            if (e_type == ET_REL) {
                fprintf(stdout, ",VALUE=%s+0x%.2X",
                        pacShStrtab + paShdr[paSym[sym].st_shndx].sh_name,
                        paSym[sym].st_value);
            } else {
                fprintf(stdout, ",ADDRESS=%.8X",
                        paSym[sym].st_value);
            }
        }
        if (paSym[sym].st_size != 0) {
            fprintf(stdout, ",SIZE=%u",
                    paSym[sym].st_size);
        }
        fputs("\n", stdout);
    }
} /* vDumpSymtab */

static void vDumpElfFile(
    ElfFile * pElfFile)
{
    unsigned entry;
    static const char aszShType[][16] = {
        "NULL",
        "PROGBITS",
        "SYMTAB",
        "STRTAB",
        "RELA",
        "HASH",
        "DYNAMIC",
        "NOTE",
        "NOBITS",
        "REL",
        "SHLIB",
        "DYNSYM",
        "Type=0000000C",
        "Type=0000000D",
        "INIT_ARRAY",
        "FINI_ARRAY",
        "PREINIT_ARRAY",
        "GROUP",
        "SYMTAB_SHNDX" };
    static const char aszPhFlags[8][4] = {
        "---", "--x", "-w-", "-wx",
        "r--", "r-x", "rw-", "rwx" };
    static const char aszNtTypes[][12] = {
        "Type 0", "PRSTATUS", "FPREGSET", "PRPSINFO", "TASKSTRUCT" };

    switch(pElfFile->Ehdr.e_machine) {
      case EM_386:
        fputs("Intel/x86", stdout);
        break;
      default:
        fputs("S/390", stdout);
    }
    if (pElfFile->Ehdr.e_ident[EI_CLASS] == ELFCLASS32)
        fputs(" 32", stdout);
    else fputs(" 64", stdout);
    fputs("-bit ELF", stdout);
    switch (pElfFile->Ehdr.e_type) {
      case ET_REL:
        fputs(" relocatable object", stdout);
        break;
      case ET_EXEC:
        fputs(" executable program", stdout);
        break;
      case ET_DYN:
        fputs(" shared object", stdout);
        break;
      case ET_CORE:
        fputs(" core dump", stdout);
        break;
    }
    fprintf(stdout, " file \"%s\"\n",
            pElfFile->pszPathName);
    switch(pElfFile->Ehdr.e_type) {
      case ET_EXEC:
      case ET_DYN:
        fprintf(stdout, "   Entry=%.8X\n", pElfFile->Ehdr.e_entry);
    }
    if (pElfFile->Ehdr.e_phnum != 0) {
        fprintf(stdout, "\nProgram header (%u entries)\n",
                pElfFile->Ehdr.e_phnum);
        fputs("type     offset   vaddr    paddr    filesz   memsz    flags    align\n", stdout);
        for (entry = 0; entry < pElfFile->Ehdr.e_phnum; ++entry) {
            fprintf(stdout, "%.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X\n",
                    pElfFile->paPhdr[entry].p_type,
                    pElfFile->paPhdr[entry].p_offset,
                    pElfFile->paPhdr[entry].p_vaddr,
                    pElfFile->paPhdr[entry].p_paddr,
                    pElfFile->paPhdr[entry].p_filesz,
                    pElfFile->paPhdr[entry].p_memsz,
                    pElfFile->paPhdr[entry].p_flags,
                    pElfFile->paPhdr[entry].p_align);
        }
    }

    for (entry = 0; entry < pElfFile->Ehdr.e_shnum; ++entry) {
        fprintf(stdout, "\nSection %u", entry);
        if (pElfFile->paShdr[entry].sh_name != 0) {
            fprintf(stdout, "  %s",
                    (char *)(pElfFile->papSection[pElfFile->Ehdr.e_shstrndx]) +
                    pElfFile->paShdr[entry].sh_name);
        }
        fputs("  ", stdout);
        if (pElfFile->paShdr[entry].sh_type <= SHT_SYMTAB_SHNDX)
            fputs(aszShType[pElfFile->paShdr[entry].sh_type], stdout);
        else switch (pElfFile->paShdr[entry].sh_type) {
          case SHT_GNU_verdef:
            fputs("GNU_verdef", stdout);
            break;
          case SHT_GNU_verneed:
            fputs("GNU_verneed", stdout);
            break;
          case SHT_GNU_versym:
            fputs("GNU_versym", stdout);
            break;
          default:
            fprintf(stdout, "Type=%.8X", pElfFile->paShdr[entry].sh_type);
        }
        if (pElfFile->paShdr[entry].sh_flags & SHF_ALLOC)
            fputs(",ALLOC", stdout);
        if (pElfFile->paShdr[entry].sh_flags & SHF_WRITE)
            fputs(",WRITE", stdout);
        if (pElfFile->paShdr[entry].sh_flags & SHF_EXECINSTR)
            fputs(",EXEC", stdout);
        if (pElfFile->paShdr[entry].sh_size != 0) {
            if (pElfFile->paShdr[entry].sh_entsize != 0) {
                fprintf(stdout, ",ENTRIES=%u",
                        pElfFile->paShdr[entry].sh_size /
                        pElfFile->paShdr[entry].sh_entsize);
            } else fprintf(stdout, ",SIZE=0x%.4X(%u)",
                           pElfFile->paShdr[entry].sh_size,
                           pElfFile->paShdr[entry].sh_size);
        }
        if (pElfFile->paShdr[entry].sh_addr != 0) {
            fprintf(stdout, ",ADDRESS=0x%.8X",
                    pElfFile->paShdr[entry].sh_addr);
        } else if (pElfFile->paShdr[entry].sh_offset != 0) {
            fprintf(stdout, ",OFFSET=0x%.4X",
                    pElfFile->paShdr[entry].sh_offset);
        }
        if ((pElfFile->Ehdr.e_type == ET_REL) &&
            (pElfFile->paShdr[entry].sh_addralign > 1)) {
            fprintf(stdout, ",ALIGN=%u",
                    pElfFile->paShdr[entry].sh_addralign);
        }
        if (pElfFile->paShdr[entry].sh_link != 0) {
            fprintf(stdout, ",LINK=%u",
                    pElfFile->paShdr[entry].sh_link);
        }
        if (pElfFile->paShdr[entry].sh_info != 0) {
            fprintf(stdout, ",INFO=%u",
                    pElfFile->paShdr[entry].sh_info);
        }
        fputs("\n", stdout);
        if (pElfFile->paShdr[entry].sh_size == 0)
            continue;
        switch (pElfFile->paShdr[entry].sh_type) {
          case SHT_NOBITS:
          case SHT_STRTAB:
            break;
          case SHT_REL:
            /* This avoids dumping .rel.stab */
            if (pElfFile->paShdr[pElfFile->paShdr[entry].sh_info].sh_flags & SHF_ALLOC)
                vDumpRel(pElfFile->papSection[entry],
                         pElfFile->paShdr[entry].sh_size /
                         pElfFile->paShdr[entry].sh_entsize,
                         pElfFile->papSection[pElfFile->paShdr[entry].sh_link],
                         pElfFile->papSection[pElfFile->paShdr[pElfFile->paShdr[entry].sh_link].sh_link],
                         pElfFile->paShdr,
                         pElfFile->papSection[pElfFile->Ehdr.e_shstrndx]);
            break;
          case SHT_RELA:
            /* This avoids dumping .rela.stab */
            if (pElfFile->paShdr[pElfFile->paShdr[entry].sh_info].sh_flags & SHF_ALLOC)
                vDumpRela(pElfFile->papSection[entry],
                          pElfFile->paShdr[entry].sh_size /
                          pElfFile->paShdr[entry].sh_entsize,
                          pElfFile->papSection[pElfFile->paShdr[entry].sh_link],
                          pElfFile->papSection[pElfFile->paShdr[pElfFile->paShdr[entry].sh_link].sh_link],
                          pElfFile->paShdr,
                          pElfFile->papSection[pElfFile->Ehdr.e_shstrndx]);
            break;
          case SHT_DYNSYM:
          case SHT_SYMTAB:
            vDumpSymtab(pElfFile->Ehdr.e_type,
                        pElfFile->papSection[entry],
                        pElfFile->paShdr[entry].sh_size /
                        pElfFile->paShdr[entry].sh_entsize,
                        pElfFile->papSection[pElfFile->paShdr[entry].sh_link],
                        pElfFile->paShdr,
                        pElfFile->papSection[pElfFile->Ehdr.e_shstrndx]);
            break;
          case SHT_PROGBITS:
            if (strcmp((char *)(pElfFile->papSection[pElfFile->Ehdr.e_shstrndx]) +
                       pElfFile->paShdr[entry].sh_name, ".stab") == 0)
                continue; /* .stab - needs special formatting */
          default:
            vDumpProgbits(pElfFile->papSection[entry],
                          (pElfFile->paShdr[entry].sh_addr != 0)
                          ? pElfFile->paShdr[entry].sh_addr
                          : pElfFile->paShdr[entry].sh_offset,
                          pElfFile->paShdr[entry].sh_size);
        }
    }

    if (pElfFile->papSegment != NULL) { /* Dump the core file */
        for (entry = 0; entry < pElfFile->Ehdr.e_phnum; ++entry) {
            switch (pElfFile->paPhdr[entry].p_type) {
              case PT_LOAD:
                fprintf(stdout, "\n%.8X-%.8X flags:%s\n",
                        pElfFile->paPhdr[entry].p_vaddr,
                        pElfFile->paPhdr[entry].p_vaddr +
                        pElfFile->paPhdr[entry].p_memsz - 1,
                        aszPhFlags[pElfFile->paPhdr[entry].p_flags & 7U]);
                vDumpProgbits(pElfFile->papSegment[entry],
                              pElfFile->paPhdr[entry].p_vaddr,
                              pElfFile->paPhdr[entry].p_filesz);
                break;
              case PT_NOTE:
                if (pElfFile->paPhdr[entry].p_filesz != 0) {
                    const char * pacNote;
                    Elf32_Off    pos;
                    Elf32_Note   note;
                    pacNote = pElfFile->papSegment[entry];
                    pos     = 0;
                    do {
                        memcpy(&note, pacNote + pos, sizeof(note));
                        if (pElfFile->Ehdr.e_ident[EI_DATA] != BYTE_ORDER) {
                            SWAP4(&(note.n_key_size));
                            SWAP4(&(note.n_data_size));
                            SWAP4(&(note.n_type));
                        }
                        pos += sizeof(note);
                        fprintf(stdout, "\n%.*s ",
                                (int)(note.n_key_size),
                                pacNote + pos);
                        pos += note.n_key_size;
                        if (note.n_type <= NT_TASKSTRUCT)
                            fprintf(stdout, "%s:\n",
                                    aszNtTypes[note.n_type]);
                        else fprintf(stdout, "Type %u\n", note.n_type);
                        vDumpProgbits(pacNote + pos,
                                      pElfFile->paPhdr[entry].p_offset + pos,
                                      note.n_data_size);
                        pos += note.n_data_size;
                    } while (pos < pElfFile->paPhdr[entry].p_filesz);
                }
            }
        }
    }
} /* vDumpElfFile */

int main(
    int argc,
    char * argv[])
{
    int     iReturn = 0;
    int     i;
    ElfFile elfFile;

    memset(&elfFile, 0, sizeof(elfFile));
    for (i = 1; i < argc; ++i)
        if (elfFile.pszPathName == NULL)
            elfFile.pszPathName = argv[i];
        else break;
    if ((i < argc) || (elfFile.pszPathName == NULL)) {
        fprintf(stderr, "Usage: %s <elf-file>\n", argv[0]);
        iReturn = 1;
    }

    if (iReturn == 0)
        iReturn = iLoadElfFile(&elfFile);
    if (elfFile.pFile != NULL) {
        fclose(elfFile.pFile);
        elfFile.pFile = NULL;
    }

    if (iReturn == 0)
        vDumpElfFile(&elfFile);

    return iReturn;
} /* main */
/**************************** elfdump.c *******************************/
