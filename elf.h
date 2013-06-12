#ifndef ELF_H
#define ELF_H

#include <sys/types.h>

typedef struct elf elf_t;

elf_t* elf_new  ();
void   elf_del  (elf_t* elf);
void   elf_begin(elf_t* elf, int fd);

size_t elf_entry(elf_t* elf);             // entry point
char*  elf_str  (elf_t* elf, size_t off); // string at memory address

#endif
