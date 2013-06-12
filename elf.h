#ifndef ELF_H
#define ELF_H

#include <sys/types.h>

typedef struct elf elf_t;

elf_t* elf_new  ();
void   elf_begin(elf_t* elf, int fd);
size_t elf_entry(elf_t* elf);

#endif
