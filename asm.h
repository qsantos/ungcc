#ifndef ASM_H
#define ASM_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

enum opcode
{
	UNK,  // unknown
	NOP,  RET, LEAVE, HLT,
	PUSH, POP,
	JMP,
	JE,   JNE,
	JA,   JAE,
	JB,   JBE,
	JS,   JNS,
	JL,   JLE,
	JG,   JGE,
	CALL,
	NOT,  NEG,
	XCHG,
	ADD,  SUB, MUL, DIV,
	AND,  OR,  XOR,
	SAR,  SAL, SHR, SHL,
	TEST, CMP,
	MOV,  LEA,
};

typedef unsigned char reg;
typedef ssize_t       im;
typedef struct
{
	size_t base;  // base register
	size_t idx;   // index register
	size_t scale; // scale factor
	size_t disp;  // displacement
} addr;

enum op_type
{
	REG,
	IM,
	ADDR,
};

typedef struct 
{
	enum op_type t;
	union
	{
		reg  reg;
		im   im;
		addr addr;
	} v;
	const char* symbol;
} op;

struct instr
{
	// dump information
	size_t offset; // offset of the instruction
	char*  orig;   // original instruction in dump

	// instruction
	char* label;
	enum opcode op; // opcode
	size_t      s;  // 8, 16 or 32 bit operation
	op          a;  // first operand
	op          b;  // second operand (destination)

	// execution path information
	bool function;
	bool branch;
};

struct asm
{
	struct instr* i;
	size_t        a;
	size_t        n;
};

// constructor, destructor
void asm_new(struct asm* asm);
void asm_del(struct asm* asm);

// instruction building
struct instr* asm_next(struct asm* asm, size_t offset, char* orig, char* label);
void asm_set_reg (op* op, reg reg);
void asm_set_im  (op* op, im im);
void asm_set_addr(op* op, reg base, reg idx, im scale, im disp);

// other
int print_reg  (char* str, size_t size, reg reg, size_t s);
int print_hex  (char* str, size_t size, im im);
int print_op   (char* str, size_t size, op* op, size_t s);
int print_instr(char* str, size_t size, struct instr* i);

void printf_instr(FILE* f, struct instr* i);

struct instr* offset2instr(struct asm* asm, size_t offset);

#endif
