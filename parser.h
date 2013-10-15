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

#ifndef PARSER_H
#define PARSER_H

#include "elist.h"
#include "elf.h"

char* read_register(expr_reg_type_t* dst, int* sz, elf_t* elf, char* str);
char* read_operand (expr_t** dst, int* sz,         elf_t* elf, char* str);
void  read_instr   (elist_t* dst, address_t of,    elf_t* elf, char* str);
void  read_file    (elist_t* dst,                  elf_t* elf, FILE* f);

#endif
