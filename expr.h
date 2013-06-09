#ifndef EXPR_H
#define EXPR_H

#include <sys/types.h>
#include <stdbool.h>
      
// operand
typedef enum
{
	R_IZ,                   // null pseudo-register
	R_AX, R_BX, R_CX, R_DX, // usual registers (or low parts)
	R_AL, R_BL, R_CL, R_DL, // low parts
	R_AH, R_BH, R_CH, R_DH, // high parts
	R_SP, R_BP,             // segment and base pointers
	R_SI, R_DI,             // segment and data indexes

	R_FL, // flags

	// FPU registers
	R_ST0, R_ST1, R_ST2, R_ST3,
	R_ST4, R_ST5, R_ST6, R_ST7,
} reg_t;
typedef ssize_t im_t;  // immediate value
typedef struct         // value at address
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

	char* symbol;
} operand_t;

typedef enum
{
	// operand: register, immediate or address
	E_OPERAND,

	// zeroary
	E_NOP,  E_RET, E_HLT,

	// unary
	E_PUSH, E_POP,                       // statments
	E_JMP,                               // inconditionnal jump
	E_JE, E_JS, E_JA, E_JB, E_JL, E_JG,  // test statments
	E_JNE,E_JNS,E_JAE,E_JBE,E_JLE,E_JGE, // test statments
	E_CALL,                              // function call
	E_NOT,  E_NEG,                       // expressions

	// binary
	E_ADD,  E_SUB, E_SBB, E_MUL, E_DIV, // mathematic operators
	E_AND,  E_OR,  E_XOR,               // logic operators
	E_SAR,  E_SAL, E_SHR, E_SHL,        // shifting operators
	E_XCHG, E_MOV, E_LEA,               // affectation
} etype_t;

// recursive structure
typedef struct expr expr_t;

struct expr
{
	etype_t type;
	union
	{
		operand_t                      op;
		char*                          fun;
		struct {expr_t* a;           } uni;
		struct {expr_t* a; expr_t* b;} bin;
	} v;
	char* label;

	// hierarchy information
	bool    isFun;
	bool    endBlck;
	expr_t* next;
	expr_t* branch;

	bool visited;
};

expr_t* e_new();
void    e_del(expr_t* e);

// operand
expr_t* e_op_reg (reg_t reg);
expr_t* e_op_im  (im_t im);
expr_t* e_op_addr(reg_t base, reg_t idx, im_t scale, im_t disp);

// zeroary
expr_t* e_nop  ();
expr_t* e_ret  ();
expr_t* e_hlt  ();

// unary
expr_t* e_push(expr_t* a); expr_t* e_pop(expr_t* a);
expr_t* e_jmp (expr_t* a);
expr_t* e_je  (expr_t* a); expr_t* e_jne(expr_t* a);
expr_t* e_js  (expr_t* a); expr_t* e_jns(expr_t* a);
expr_t* e_ja  (expr_t* a); expr_t* e_jae(expr_t* a);
expr_t* e_jb  (expr_t* a); expr_t* e_jbe(expr_t* a);
expr_t* e_jl  (expr_t* a); expr_t* e_jle(expr_t* a);
expr_t* e_call(expr_t* a);
expr_t* e_jg  (expr_t* a); expr_t* e_jge(expr_t* a);
expr_t* e_not (expr_t* a); expr_t* e_neg(expr_t* b);

// binary
expr_t* e_add (expr_t* a, expr_t* b);
expr_t* e_sub (expr_t* a, expr_t* b);
expr_t* e_sbb (expr_t* a, expr_t* b);
expr_t* e_mul (expr_t* a, expr_t* b);
expr_t* e_div (expr_t* a, expr_t* b);
expr_t* e_and (expr_t* a, expr_t* b);
expr_t* e_or  (expr_t* a, expr_t* b);
expr_t* e_xor (expr_t* a, expr_t* b);
expr_t* e_sar (expr_t* a, expr_t* b);
expr_t* e_sal (expr_t* a, expr_t* b);
expr_t* e_shr (expr_t* a, expr_t* b);
expr_t* e_shl (expr_t* a, expr_t* b);
expr_t* e_xchg(expr_t* a, expr_t* b);
expr_t* e_mov (expr_t* a, expr_t* b);
expr_t* e_lea (expr_t* a, expr_t* b);

void reset_visited(expr_t* e);
int  cmp_op       (operand_t* a, operand_t* b);

#endif
