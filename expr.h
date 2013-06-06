#ifndef EXPR_H
#define EXPR_H

#include "asm.h"

// recusrive structure
typedef struct expr expr_t;

typedef struct
{
	expr_t* a;
	expr_t* b;
} expr_bin_t;
typedef char* function_t;

struct expr
{
	enum
	{
		I_ASM,     // unparsed assembly
		I_OPERAND, // operand: register, immediate or address
		I_FUNCT,   // function call

		// instructions
		I_PUSH, I_POP,

		// tests
		I_TEST, I_CMP,
		I_JE, I_JS, I_JA, I_JB, I_JL, I_JG,
		I_JNE,I_JNS,I_JAE,I_JBE,I_JLE,I_JGE,

		// unary expressions
		I_NOT,  I_NEG,

		// binary expressions
		I_ADD,  I_SUB, I_MUL, I_DIV,
		I_AND,  I_OR,  I_XOR,
		I_SAR,  I_SAL, I_SHR, I_SHL,

		// affectation
		I_XCHG, I_MOV, I_LEA,
	} type;
	union
	{
		operand_t  op;
		expr_bin_t bin;
		function_t fun;
	} v;
};

// recursive structure
typedef struct block block_t;

struct block
{
	// block information
	expr_t* code;
	size_t  size;

	// branching information
	block_t* next;
	block_t* branch;

	// displaying information
	bool   visited;
	double x;
	double y;
	double h;
	double w;
};

#endif
