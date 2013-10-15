/*\
 *  Binary analyzer for decompilation and forensics
 *  Copyright (C) 2013  Quentin SANTOS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/

#ifndef EXPR_H
#define EXPR_H

#include <sys/types.h>
#include <stdbool.h>

typedef struct expr expr_t;

// some basic types
#ifndef ELF_H // TODO
typedef unsigned long long address_t;
typedef   signed long long offset_t;
typedef   signed long long value_t;
#endif

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
} expr_reg_type_t;
typedef struct
{
	expr_reg_type_t t;
	expr_t*    last; // address of last reg affectation
} expr_reg_t;
// immediate value
typedef struct
{
	value_t     v;   // actual value
	function_t* sym; // symbol information
	char*       str; // string value
} expr_im_t;
// indirect addressing
typedef struct
{
	expr_reg_type_t base;  // base register
	expr_reg_type_t idx;   // index register
	value_t    scale; // scale factor
	value_t    disp;  // displacement
} expr_addr_t;
// function call
typedef struct
{
	expr_t*  f; // function address
	size_t   argc; // argument count
	expr_t** argv; // argument values
	bool     fast; // fast parameter passing
} expr_call_t;
// unary
typedef struct
{
	expr_t* a;
} expr_uni_t;
// binary
typedef struct
{
	expr_t* a;
	expr_t* b;
} expr_bin_t;
// comparison test
typedef enum
{
	T_E, T_S, T_A, T_B,
	T_NE,T_NS,T_AE,T_BE,
} expr_test_type_t;
typedef struct
{
	expr_test_type_t t;
	expr_t* a;
} expr_test_t;

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
} expr_type_t;

struct expr
{
	expr_type_t type;
	union
	{
		char*  unk;
		expr_reg_t  reg;
		expr_im_t   im;
		expr_addr_t addr;
		expr_call_t call;
		expr_uni_t  uni;
		expr_bin_t  bin;
		expr_test_t test;
	} v;

	// hierarchy information
	bool    isFun;
	bool    endBlck;
	expr_t* next;
	expr_t* branch;

	// for MOV: number of uses of this reg affectation
	unsigned int used;

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
expr_t* e_reg (expr_reg_type_t reg);
expr_t* e_im  (value_t         im);
expr_t* e_addr(expr_reg_type_t base, expr_reg_type_t idx, value_t scale, value_t disp);

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
expr_t* e_test(expr_test_type_t t, expr_t* a);

#endif
