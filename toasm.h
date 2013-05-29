#ifndef TOASM_H
#define TOASM_H

#include "asm.h"

// reads a register from 'reg' and return its code
size_t regcode(const char* reg, const char** end);

// returns the bit-size of the next register
size_t regsize(const char* reg);

// reads an operand and fills 'op'
const char* read_op(op* op, const char* str, size_t* s);

// instruction without parameters
#define READ_INSTR0(O,N) if (strcmp(opcode,N) == 0) { i->op = O; }

// unary instruction
#define READ_INSTR1(O,N) if (strcmp(opcode,N) == 0) { i->op = O; \
	read_op(&i->a,params,&i->s); } 

// binary instruction
#define READ_INSTR2(O,N) if (strcmp(opcode,N) == 0) { i->op = O; \
	params = read_op(&i->a,params,&i->s) + 1; \
	read_op(&i->b,params,&i->s); } 

// generate 'asm' from 'f'
#include <stdio.h>
void fromfile(struct asm* asm, FILE* f);

#endif
