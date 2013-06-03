#ifndef TOASM_H
#define TOASM_H

#include "asm.h"

// reads a register from 'reg' and return its code
size_t regcode(const char* reg, const char** end);

// returns the bit-size of the next register
size_t regsize(const char* reg);

// reads an operand and fills 'op'
const char* read_op(op* op, const char* str, size_t* s);

// generate 'asm' from 'f'
#include <stdio.h>
void fromfile(struct asm* asm, FILE* f);

#endif
