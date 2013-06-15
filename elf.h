#ifndef ELF_H
#define ELF_H

#include <sys/types.h>

typedef struct elf elf_t;

// constructor, destructor, initialization
elf_t* elf_new  ();
void   elf_del  (elf_t* elf);
void   elf_begin(elf_t* elf, int fd);

// returns the entry point of the executable
size_t elf_entry(elf_t* elf);

// search for a string at address 'addr' in .rodata
// if no such string exists, return NULL
char* elf_str(elf_t* elf, size_t addr);

// returns the symbol corresponding to the function called
// through the PLT wrapper located at address 'addr'
char* elf_plt(elf_t* elf, size_t addr);

char* elf_sym(elf_t* elf, size_t addr);

#endif
