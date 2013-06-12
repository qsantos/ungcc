#include "elf.h"

#include <unistd.h>
#include <string.h>
#include <elf.h>

#include <stdio.h>
#include <stdlib.h>

#define ERR(...) { fprintf(stderr, __VA_ARGS__); exit(1); }

struct elf
{
	Elf32_Ehdr hdr;
};

elf_t* elf_new()
{
	elf_t* ret = malloc(sizeof(elf_t));
	return ret;
}

void elf_begin(elf_t* elf, int fd)
{
	// reads the ELF32 header
	lseek(fd, SEEK_SET, 0);
	read (fd, &elf->hdr, sizeof(Elf32_Ehdr));

	if (strncmp((char*) &elf->hdr.e_ident, ELFMAG, SELFMAG) != 0)
		ERR("Not an ELF file\n");

	if (elf->hdr.e_type != ET_EXEC)
		ERR("Not an executable file\n");

	if (elf->hdr.e_machine != EM_386)
		ERR("Not a x86 executable\n");

	if (elf->hdr.e_version != SYMINFO_CURRENT)
		ERR("Unknown ELF version %u\n", elf->hdr.e_version);
}

size_t elf_entry(elf_t* elf)
{
	return (size_t) elf->hdr.e_entry;
}
