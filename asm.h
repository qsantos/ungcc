#ifndef ASM_H
#define ASM_H

#include <stdlib.h>

enum opcode
{
	UNK,  // unknown
	MOV,
	LEA,
	CALL,
	JMP,
	JE,
	JNE,
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
} op;

struct instr
{
	// dump information
	size_t offset; // offset of the instruction
	char*  orig;   // original instruction in dump

	// instruction
	enum opcode op; // opcode
	size_t      s;  // 8, 16 or 32 bit operation
	op          a;  // first operand
	op          b;  // second operand (destination)

/*
	// execution information
	unsigned long next;   // next instruction   (may be zero)
	unsigned long branch; // branch instruction (may be zero)
*/
};

struct asm
{
	struct instr* i;
	size_t        a;
	size_t        n;
};

// constructor, destructor
void asm_new(struct asm* c);
void asm_del(struct asm* c);

// instruction building
struct instr* asm_next(struct asm* c, size_t offset, char* orig);
void asm_set_reg (op* op, reg reg);
void asm_set_im  (op* op, im im);
void asm_set_addr(op* op, reg base, reg idx, im scale, im disp);

// other
void asm_print(struct asm* c);

#endif
