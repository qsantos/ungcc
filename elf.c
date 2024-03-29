/*\
 *  Binary analyzer for decompilation and forensics
 *  Copyright (C) 2013  Quentin SANTOS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/

#include "elf.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <elf.h>

#define ERR(...) { fprintf(stderr, __VA_ARGS__); exit(1); }

struct elf
{
	int fd;
	Elf32_Ehdr hdr;  // ELF header

	// some section headers
	Elf32_Shdr text;
	Elf32_Shdr rodata;
	Elf32_Shdr plt;
	Elf32_Shdr dynstr;
	Elf32_Shdr dynsym;
	Elf32_Shdr rel_plt;
	Elf32_Shdr symtab;
	Elf32_Shdr strtab;

	// the symbol table is sorted for fast searching
	Elf32_Sym* sym_d;
	size_t     sym_n;
};

elf_t* elf_new()
{
	elf_t* ret = malloc(sizeof(elf_t));
	memset(ret, 0, sizeof(elf_t));
	return ret;
}

void elf_del(elf_t* elf)
{
	free(elf);
}

static int sym_cmp(const void* a, const void* b)
{
	Elf32_Sym* sa = (Elf32_Sym*) a;
	Elf32_Sym* sb = (Elf32_Sym*) b;

	if (sa->st_value < sb->st_value) return -1;
	if (sa->st_value > sb->st_value) return  1;
	                                 return  0;
}
void elf_begin(elf_t* elf, int fd)
{
	elf->fd = fd;

	Elf32_Ehdr* hdr = &elf->hdr;

	// reads the ELF32 header
	lseek(fd, 0, SEEK_SET);
	read (fd, hdr, sizeof(Elf32_Ehdr));

	// checks format
	if (strncmp((char*) &hdr->e_ident, ELFMAG, SELFMAG) != 0)
		ERR("Not an ELF file\n");

	if (hdr->e_type != ET_EXEC)
		ERR("Not an executable file\n");

	if (hdr->e_machine != EM_386)
		ERR("Not a x86 executable\n");

	if (hdr->e_version != SYMINFO_CURRENT)
		ERR("Unknown ELF version %u\n", hdr->e_version);

	if (hdr->e_shoff == 0)
		ERR("The ELF file has no section table\n");

	if (hdr->e_shstrndx == SHN_UNDEF)
		ERR("The ELF file has no .shstrtab (string) section\n");

	Elf32_Shdr shdr;

	// gets the .shstrtab section
	lseek(fd, hdr->e_shoff + hdr->e_shstrndx * sizeof(Elf32_Shdr), SEEK_SET);
	read(fd, &shdr, sizeof(Elf32_Shdr));
	Elf32_Off strtaboff = shdr.sh_offset;

	// finds the .strtab section
	for (Elf32_Half i = 0; i < hdr->e_shnum; i++)
	{
		// reads section header
		lseek(fd, hdr->e_shoff + i * sizeof(Elf32_Shdr), SEEK_SET);
		read(fd, &shdr, sizeof(Elf32_Shdr));

		// read name (expecting .rodata, .dynstr, .dynsym or .rel.plt)
		lseek(fd, strtaboff + shdr.sh_name, SEEK_SET);
		char buf[9];
		read(fd, buf, 9);

		// save in appropriate member
		if      (strcmp(buf, ".text"   ) == 0) memcpy(&elf->text,    &shdr, sizeof(Elf32_Shdr));
		else if (strcmp(buf, ".rodata" ) == 0) memcpy(&elf->rodata,  &shdr, sizeof(Elf32_Shdr));
		else if (strcmp(buf, ".plt"    ) == 0) memcpy(&elf->plt,     &shdr, sizeof(Elf32_Shdr));
		else if (strcmp(buf, ".dynstr" ) == 0) memcpy(&elf->dynstr,  &shdr, sizeof(Elf32_Shdr));
		else if (strcmp(buf, ".dynsym" ) == 0) memcpy(&elf->dynsym,  &shdr, sizeof(Elf32_Shdr));
		else if (strcmp(buf, ".rel.plt") == 0) memcpy(&elf->rel_plt, &shdr, sizeof(Elf32_Shdr));
		else if (strcmp(buf, ".strtab" ) == 0) memcpy(&elf->strtab,  &shdr, sizeof(Elf32_Shdr));
		else if (strcmp(buf, ".symtab" ) == 0)
		{
			memcpy(&elf->symtab, &shdr, sizeof(Elf32_Shdr));

			elf->sym_d = (Elf32_Sym*) malloc(shdr.sh_size);
			elf->sym_n = shdr.sh_size / sizeof(Elf32_Sym);

			lseek(fd, shdr.sh_offset, SEEK_SET);
			read(fd, elf->sym_d, shdr.sh_size);
			qsort(elf->sym_d, elf->sym_n, sizeof(Elf32_Sym), sym_cmp);
		}
	}
}

address_t elf_entry(elf_t* elf)
{
	return (address_t) elf->hdr.e_entry;
}

#define ADDCH(C) {if (n == a) { a*=2; ret = realloc(ret, a); } ret[n++] = C;}
char* elf_str(elf_t* elf, address_t addr)
{
	int         fd   = elf->fd;
	Elf32_Shdr* shdr = &elf->rodata;

	// checks that address is in .strtab
	Elf32_Addr pltaddr = shdr->sh_addr;
	if (!(pltaddr <= addr && addr < pltaddr + shdr->sh_size))
		return NULL;

	Elf32_Off off = addr - pltaddr;
	lseek(fd, shdr->sh_offset + off, SEEK_SET);
	char*  ret = malloc(1);
	size_t n   = 0;
	size_t a   = 1;

	char c;
	while (read(fd, &c, 1) && c)
		if      (c == '\n') {ADDCH('\\'); ADDCH('n');}
		else if (c == '\r') {ADDCH('\\'); ADDCH('r');}
		else if (c == '\t') {ADDCH('\\'); ADDCH('t');}
		else                 ADDCH(c)

	ADDCH(0);
	return ret;
}

char* elf_plt(elf_t* elf, address_t addr)
{
	int fd = elf->fd;

	// the address should be in .plt
	Elf32_Addr pltaddr = elf->plt.sh_addr;
	if (!(pltaddr <= addr && addr < pltaddr + elf->plt.sh_size))
		return NULL;

	Elf32_Off off = addr - pltaddr;

	// reads the reloc_offset from the PLT wrapper
	// 6 bytes for the first jmp instruction
	// 1 byte for the pushl opcode
	lseek(fd, elf->plt.sh_offset + off + 0x7, SEEK_SET);
	Elf32_Off reloc;
	read(fd, &reloc, 4);

	// the offset should be in .rel.plt
	if (reloc > elf->rel_plt.sh_size)
		return NULL;

	// plt relation information
	lseek(fd, elf->rel_plt.sh_offset + reloc, SEEK_SET);
	Elf32_Rel rel;
	read(fd, &rel, sizeof(Elf32_Rel));
	Elf32_Word symidx = rel.r_info >> 8; // gets the index of the symbol
	Elf32_Off symaddr = symidx * sizeof(Elf32_Sym); // symbol address

	// the offset should be in .dyn.sym
	if (symaddr > elf->dynsym.sh_size)
		return NULL;

	// symbol information
	lseek(fd, elf->dynsym.sh_offset + symaddr, SEEK_SET);
	Elf32_Sym sym;
	read(fd, &sym, sizeof(Elf32_Sym));
	Elf32_Off stroff = sym.st_name; // gets the string offset of the symbol name

	// the offset should be in .dyn.str
	if (stroff > elf->dynstr.sh_size)
		return NULL;

	// reads the symbol name
	lseek(fd, elf->dynstr.sh_offset + stroff, SEEK_SET);

	char*  ret = malloc(1);
	size_t n   = 0;
	size_t a   = 1;
	char c;
	while (read(fd, &c, 1))
	{
		if (n == a)
		{
			a *= 2;
			ret = realloc(ret, a);
		}
		ret[n++] = c;
		if (c == 0)
			break;
	}

	return ret;
}

char* elf_sym(elf_t* elf, address_t addr)
{
	int fd = elf->fd;

	// the address should be in .text // TODO
	Elf32_Addr textaddr = elf->text.sh_addr;
	if (!(textaddr <= addr && addr < textaddr + elf->text.sh_size))
		return NULL;

	// finds the first information about the symbol
	Elf32_Sym key;
	key.st_value = addr;
	Elf32_Sym* sym = bsearch(&key, elf->sym_d, elf->sym_n, sizeof(Elf32_Sym), sym_cmp);

	if (sym == NULL)
		return NULL;

	// finds the first right symbol
	for (; sym->st_value == addr && ELF32_ST_TYPE(sym->st_info) != STT_FUNC; sym++);
	if (sym->st_value != addr)
		return NULL;

	// reads the symbol name
	Elf32_Off stroff = sym->st_name; // gets the string offset of the symbol name
	lseek(fd, elf->strtab.sh_offset + stroff, SEEK_SET);

	char*  ret = malloc(1);
	size_t n   = 0;
	size_t a   = 1;
	char c;
	while (read(fd, &c, 1))
	{
		if (n == a)
		{
			a *= 2;
			ret = realloc(ret, a);
		}
		ret[n++] = c;
		if (c == 0)
			break;
	}

	return ret;
}
