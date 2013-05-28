#ifndef TOASM_H
#define TOASM_H

#include "asm.h"

// reads a register from stdin and return its code
size_t regcode(char* reg, char** end);

// return the bit-size of the next register
size_t regsize(const char* reg);

// reads an operand and fills 'op'
char* read_op(op* op, char* str, size_t* s);

#endif
