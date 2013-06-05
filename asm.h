#ifndef ASM_H
#define ASM_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

// opcode
typedef enum
{
	UNK,  // unknown

	// no parameter
	NOP,  RET, LEAVE, HLT,

	// unary instructions
	PUSH, POP,
	JMP,
	JE, JNE,   JA, JAE,   JB, JBE,   JS, JNS,   JL, JLE,   JG, JGE,
	CALL,
	NOT,  NEG,

	// binary instructions
	ADD,  SUB, MUL, DIV,
	AND,  OR,  XOR,
	SAR,  SAL, SHR, SHL,
	TEST, CMP,
	XCHG, MOV,  LEA,
} opcode_t;

// operand
typedef unsigned char reg_t; // value in register
typedef ssize_t       im_t;  // immediate value
typedef struct             // value at address
{
	reg_t  base;  // base register
	reg_t  idx;   // index register
	size_t scale; // scale factor
	size_t disp;  // displacement
} addr_t;
typedef struct
{
	enum
	{
		REG,  // register
		IM,   // immediate
		ADDR, // address
	} t;
	union
	{
		reg_t  reg;
		im_t   im;
		addr_t addr;
	} v;
	const char* symbol;
} operand_t;

// instruction
typedef struct
{
	// dump information
	size_t offset; // offset of the instruction
	char*  orig;   // original instruction in dump

	// instruction
	char* label;
	opcode_t op; // opcode
	size_t      s;  // 8, 16 or 32 bit operation
	operand_t        a;  // first operand
	operand_t        b;  // second operand (destination)

	// execution path information
	bool function;
	bool branch;
} instr_t;

typedef struct
{
	instr_t* i;
	size_t        a;
	size_t        n;
} asm_t;

// constructor, destructor
void asm_new(asm_t* asm);
void asm_del(asm_t* asm);

// instruction building
instr_t* asm_next    (asm_t* asm, size_t offset, char* orig, char* label);
instr_t* asm_find_at (asm_t* asm, size_t offset);
void     asm_set_reg (operand_t* op, reg_t reg);
void     asm_set_im  (operand_t* op, im_t im);
void     asm_set_addr(operand_t* op, reg_t base, reg_t idx, im_t scale, im_t disp);

// parse information from string
const char* read_register(reg_t*     dst, size_t* sz, const char* str);
const char* read_operand (operand_t* dst, size_t* sz, const char* str);
void        read_file    (asm_t*     dst,             FILE* f);

// printing
int  print_reg   (char* str, size_t size, reg_t reg, size_t s);
int  print_hex   (char* str, size_t size, im_t im);
int  print_op    (char* str, size_t size, operand_t* op, size_t s);
int  print_instr (char* str, size_t size, instr_t* i);
void fprint_instr(FILE* f, instr_t* i);

#endif
