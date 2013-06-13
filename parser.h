#ifndef PARSER_H
#define PARSER_H

#include "elist.h"
#include "elf.h"

char* read_register(rtype_t* dst, size_t* sz, elf_t* elf, char* str);
char* read_operand (expr_t** dst, size_t* sz, elf_t* elf, char* str);
void  read_instr   (elist_t* dst, size_t  of, elf_t* elf, char* str);
void  read_file    (elist_t* dst,             elf_t* elf, FILE* f);

#endif
