#include "elf.h"

#include <unistd.h>
#include <string.h>
#include <elf.h>

#include <stdio.h>
#include <stdlib.h>

#define ERR(...) { fprintf(stderr, __VA_ARGS__); exit(1); }

struct elf
{
	int fd;
	Elf32_Ehdr hdr;  // ELF header
	Elf32_Shdr shdr; // section header (should be .strtab)
};

elf_t* elf_new()
{
	elf_t* ret = malloc(sizeof(elf_t));
	return ret;
}

void elf_del(elf_t* elf)
{
	free(elf);
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

	Elf32_Shdr* shdr = &elf->shdr;

	// gets the .shstrtab section
	lseek(fd, hdr->e_shoff + hdr->e_shstrndx * sizeof(Elf32_Shdr), SEEK_SET);
	read(fd, shdr, sizeof(Elf32_Shdr));
	Elf32_Off strtaboff = shdr->sh_offset;

	// finds the .strtab section
	for (Elf32_Half i = 0; i < hdr->e_shnum; i++)
	{
		// reads section header
		lseek(fd, hdr->e_shoff + i * sizeof(Elf32_Shdr), SEEK_SET);
		read(fd, shdr, sizeof(Elf32_Shdr));

		// check if name is ".rodata"
		const char* sectname = ".rodata";
		lseek(fd, strtaboff + shdr->sh_name, SEEK_SET);
		size_t n = strlen(sectname);
		char buf[n+1];
		read(fd, buf, n+1);

		if (strcmp(buf, sectname) == 0)
			break;
	}
}

size_t elf_entry(elf_t* elf)
{
	return (size_t) elf->hdr.e_entry;
}

char* elf_str(elf_t* elf, size_t off)
{
	int         fd   = elf->fd;
	Elf32_Shdr* shdr = &elf->shdr;

	// checks that address is in .strtab
	size_t addr = shdr->sh_addr;
	printf("%#x, %#x -> %#x\n", addr, shdr->sh_size, off);
	if (!(addr <= off && off < addr + shdr->sh_size))
		return NULL;

	lseek(fd, off - addr + shdr->sh_offset, SEEK_SET);
	char*  ret = malloc(1);
	size_t n   = 0;
	size_t a   = 1;
	char   c;
	while (read(fd, &c, 1))
	{
		if (n == a)
		{
			a  *= 2;
			ret = realloc(ret, a);
		}
		ret[n++] = c;

		if (c == 0)
			break;
	}
	return ret;
}
