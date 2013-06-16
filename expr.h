#ifndef EXPR_H
#define EXPR_H

#include <sys/types.h>
#include <stdbool.h>

typedef struct expr expr_t;

#include "flist.h"

// register
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

	N_REG,
} rtype_t;
typedef struct
{
	rtype_t t;
	expr_t* last; // address of last reg affectation
} reg_t;
// immediate value
typedef struct
{
	ssize_t     v;   // actual value
	function_t* sym; // symbol information
	char*       str; // string value
} im_t;
// indirect addressing
typedef struct
{
	rtype_t base;  // base register
	rtype_t idx;   // index register
	size_t  scale; // scale factor
	ssize_t disp;  // displacement
} addr_t;
// function call
typedef struct
{
	expr_t*  f; // function address
	size_t   argc; // argument count
	expr_t** argv; // argument values
	bool     fast; // fast parameter passing
} call_t;
// unary
typedef struct
{
	expr_t* a;
} uni_t;
// binary
typedef struct
{
	expr_t* a;
	expr_t* b;
} bin_t;
// comparison test
typedef enum
{
	T_E, T_S, T_A, T_B,
	T_NE,T_NS,T_AE,T_BE,
} ttype_t;
typedef struct
{
	ttype_t t;
	expr_t* a;
} test_t;

typedef enum
{
	// unknown
	E_UNK,

	// register, immediate or address
	E_REG, E_IM, E_ADDR,

	// zeroary
	E_NOP,  E_RET, E_HLT,

	// function call
	E_CALL,

	// unary
	E_PUSH, E_POP, // statments
	E_NOT,  E_NEG, // expressions

	// binary
	E_JMP,  E_JXX,                      // jumps (JMP ignores 'b')
	E_ADD,  E_SUB, E_SBB, E_MUL, E_DIV, // mathematic operators
	E_AND,  E_OR,  E_XOR,               // logic operators
	E_SAR,  E_SAL, E_SHR, E_SHL,        // shifting operators
	E_XCHG, E_MOV, E_LEA,               // affectation

	E_TEST,
} etype_t;

struct expr
{
	etype_t type;
	union
	{
		char*  unk;
		reg_t  reg;
		im_t   im;
		addr_t addr;
		call_t call;
		uni_t  uni;
		bin_t  bin;
		test_t test;
	} v;

	// hierarchy information
	bool    isFun;
	bool    endBlck;
	expr_t* next;
	expr_t* branch;

	// for MOV: number of uses of this reg affectation
	size_t used;

	bool visited;
};

// generic operations
expr_t* e_new       ();
void    e_del       (expr_t* e, bool keep);
expr_t* e_cpy       (expr_t* e);
void    e_rstvisited(expr_t* e);
int     e_cmp       (expr_t* a, expr_t* b);

// = now only specific constructos =

// unknown
expr_t* e_unk(char* comment);

// register, immediate, address
expr_t* e_reg (rtype_t reg);
expr_t* e_im  (ssize_t im);
expr_t* e_addr(rtype_t base, rtype_t idx, size_t scale, ssize_t disp);

// zeroary
expr_t* e_nop();
expr_t* e_ret();
expr_t* e_hlt();

// function call
expr_t* e_call(expr_t* a);

// unary
expr_t* e_push(expr_t* a); expr_t* e_pop(expr_t* a);
expr_t* e_je  (expr_t* a); expr_t* e_jne(expr_t* a);
expr_t* e_js  (expr_t* a); expr_t* e_jns(expr_t* a);
expr_t* e_ja  (expr_t* a); expr_t* e_jae(expr_t* a);
expr_t* e_jb  (expr_t* a); expr_t* e_jbe(expr_t* a);
expr_t* e_not (expr_t* a); expr_t* e_neg(expr_t* b);

// binary
expr_t* e_jmp (expr_t* a); // b = NULL
expr_t* e_jxx (expr_t* a, expr_t* b);
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

// test
expr_t* e_test(ttype_t t, expr_t* a);

#endif
