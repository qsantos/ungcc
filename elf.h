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

#ifndef ELF_H
#define ELF_H

#include <sys/types.h>

typedef struct elf elf_t;

// TODO: duplicate from expr.h
#ifndef EXPR_H
typedef unsigned long long address_t;
typedef   signed long long offset_t;
typedef   signed long long value_t;
#endif

// constructor, destructor, initialization
elf_t* elf_new  ();
void   elf_del  (elf_t* elf);
void   elf_begin(elf_t* elf, int fd);

// returns the entry point of the executable
address_t elf_entry(elf_t* elf);

// search for a string at address 'addr' in .rodata
// if no such string exists, return NULL
char* elf_str(elf_t* elf, address_t addr);

// returns the symbol corresponding to the function called
// through the PLT wrapper located at address 'addr'
char* elf_plt(elf_t* elf, address_t addr);

char* elf_sym(elf_t* elf, address_t addr);

#endif
