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

#include "expr.h"

#include <stdlib.h>
#include <string.h>

expr_t* e_new()
{
	expr_t* ret = malloc(sizeof(expr_t));
	ret->isFun   = false;
	ret->endBlck = false;
	ret->next    = NULL;
	ret->branch  = NULL;
	ret->used    = 0;
	ret->visited = false;
	return ret;
}

void e_del(expr_t* e, bool keep)
{
	if (e == NULL) return;

	switch (e->type)
	{
	case E_UNK:
		free(e->v.unk);
		break;

	case E_REG:
		break;
	case E_IM:
		free(e->v.im.str);
		break;
	case E_ADDR:
		break;

	// zeroary
	case E_NOP:
	case E_RET:
	case E_HLT:
		break;

	// function call
	case E_CALL:
		e_del(e->v.call.f, false);
		for (size_t i = 0; i < e->v.call.argc; i++)
			e_del(e->v.call.argv[i], false);
		break;

	// unary
	case E_PUSH: case E_POP:
	case E_NOT:  case E_NEG:
		e_del(e->v.uni.a, false);
		break;

	// binary
	case E_JMP:  case E_JXX:
	case E_ADD:  case E_SUB: case E_SBB: case E_MUL: case E_DIV:
	case E_AND:  case E_OR:  case E_XOR:
	case E_SAR:  case E_SAL: case E_SHR: case E_SHL:
	case E_XCHG: case E_MOV: case E_LEA:
		e_del(e->v.bin.a, false);
		e_del(e->v.bin.b, false);
		break;
	
	case E_TEST:
		e_del(e->v.test.a, false);
		break;
	}
	
	if (!keep)
		free(e);
}

expr_t* e_cpy(expr_t* e)
{
	if (e == NULL) return NULL;

	expr_t* ret = e_new();
	ret->type = e->type;
	switch (e->type)
	{
	case E_UNK:
		ret->v.unk = strdup(e->v.unk);
		break;

	case E_REG:
		ret->v.reg.t    = e->v.reg.t;
		ret->v.reg.last = e->v.reg.last;
		break;
	case E_IM:
		ret->v.im.v   = e->v.im.v;
		ret->v.im.sym = e->v.im.sym;
		break;
	case E_ADDR:
		ret->v.addr.base  = e->v.addr.base;
		ret->v.addr.idx   = e->v.addr.idx;
		ret->v.addr.scale = e->v.addr.scale,
		ret->v.addr.disp  = e->v.addr.disp;

	// zeroary
	case E_NOP:
	case E_RET:
	case E_HLT:
		break;

	// function call
	case E_CALL:
		ret->v.call.f = e_cpy(e->v.call.f);
		size_t argc = e->v.call.argc;
		ret->v.call.argc = argc;
		ret->v.call.argv = (expr_t**) malloc(argc * sizeof(expr_t*));
		memcpy(ret->v.call.argv, e->v.call.argv, argc * sizeof(expr_t*));
		break;

	// unary
	case E_PUSH: case E_POP:
	case E_NOT:  case E_NEG:
		ret->v.uni.a = e_cpy(e->v.uni.a);
		break;

	// binary
	case E_JMP:  case E_JXX:
	case E_ADD:  case E_SUB: case E_SBB: case E_MUL: case E_DIV:
	case E_AND:  case E_OR:  case E_XOR:
	case E_SAR:  case E_SAL: case E_SHR: case E_SHL:
	case E_XCHG: case E_MOV: case E_LEA:
		ret->v.bin.a = e_cpy(e->v.bin.a);
		ret->v.bin.b = e_cpy(e->v.bin.b);
		break;
	
	case E_TEST:
		ret->v.test.t = e->v.test.t;
		ret->v.test.a = e_cpy(e->v.test.a);
	}
	return ret;
}

void e_rstvisited(expr_t* e)
{
	if (e == NULL || !e->visited)
		return;
	e->visited = false;

	e_rstvisited(e->next);
	e_rstvisited(e->branch);
}

#define CMP1(T) case T: return e_cmp(a->v.uni.a, b->v.uni.a);
#define CMP2(T) case T: return e_cmp(a->v.bin.a, b->v.bin.a) || e_cmp(a->v.bin.b, b->v.bin.b);

int e_cmp(expr_t* a, expr_t* b)
{
	if (a == NULL && b == NULL) return 0;
	if (a == NULL || b == NULL) return 1;

	if (a->type != b->type)
		return 1;
	switch (a->type)
	{
	case E_UNK:
		return 1;

	case E_REG:
		return a->v.reg.t == b->v.reg.t ? 0 : 1;
	case E_IM:
		return a->v.im.v == b->v.im.v ? 0 : 1;
	case E_ADDR:
		return a->v.addr.base  == b->v.addr.base  &&
		       a->v.addr.idx   == b->v.addr.idx   &&
		       a->v.addr.scale == b->v.addr.scale &&
		       a->v.addr.disp  == b->v.addr.disp ? 0 : 1;

	// zeroary
	case E_NOP:
	case E_RET:
	case E_HLT:
		return 0;

	// function call
	case E_CALL:
		if (e_cmp(a->v.call.f, b->v.call.f))
			return 1;
		size_t argc = a->v.call.argc;
		if (argc != b->v.call.argc)
			return 1;
		for (size_t i = 0; i < argc; i++)
			if (e_cmp(a->v.call.argv[i], b->v.call.argv[i]))
				return 1;
		break;

// TODO
	// unary
	CMP1(E_PUSH); CMP1(E_POP);
	CMP1(E_NOT);  CMP1(E_NEG);

	// binary
	CMP2(E_JMP);  CMP2(E_JXX);
	CMP2(E_ADD);  CMP2(E_SUB); CMP2(E_SBB); CMP2(E_MUL); CMP2(E_DIV);
	CMP2(E_AND);  CMP2(E_OR);  CMP2(E_XOR);
	CMP2(E_SAR);  CMP2(E_SAL); CMP2(E_SHR); CMP2(E_SHL);
	CMP2(E_XCHG); CMP2(E_MOV); CMP2(E_LEA);

	case E_TEST:
		return a->v.test.t == b->v.test.t ? e_cmp(a->v.test.a, b->v.test.a) : 0;
	}
	return 1;
}

expr_t* e_unk(char* comment)
{
	expr_t* ret = e_new();
	ret->type = E_UNK;
	ret->v.unk = comment;
	return ret;
}

// register
expr_t* e_reg(expr_reg_type_t reg)
{
	expr_t* ret = e_new();
	ret->type       = E_REG;
	ret->v.reg.t    = reg;
	ret->v.reg.last = NULL;
	return ret;
}

// immediate
expr_t* e_im(ssize_t im)
{
	expr_t* ret = e_new();
	ret->type     = E_IM;
	ret->v.im.v   = im;
	ret->v.im.sym = NULL;
	ret->v.im.str = NULL;
	return ret;
}

// address
expr_t* e_addr(expr_reg_type_t base, expr_reg_type_t idx, size_t scale, ssize_t disp)
{
	expr_t* ret = e_new();
	ret->type         = E_ADDR;
	ret->v.addr.base  = base;
	ret->v.addr.idx   = idx;
	ret->v.addr.scale = scale;
	ret->v.addr.disp  = disp;
	return ret;
}

// zeroary
#define E_ZER(N, T) expr_t* e_##N() \
{ \
	expr_t* ret = e_new(); \
	ret->type = T; \
	return ret; \
}
E_ZER(nop, E_NOP)
E_ZER(ret, E_RET)
E_ZER(hlt, E_HLT)

expr_t* e_call(expr_t* a)
{
	expr_t* ret = e_new();
	ret->type = E_CALL;
	ret->v.call.f    = a;
	ret->v.call.argc = 0;
	ret->v.call.argv = NULL;
	ret->v.call.fast = false;
	return ret;
}

// unary
#define E_UNI(N, T) expr_t* e_##N(expr_t* a) \
{ \
	expr_t* ret = e_new(); \
	ret->type = T; \
	ret->v.uni.a = a; \
	return ret; \
}
E_UNI(push, E_PUSH) E_UNI(pop, E_POP)
E_UNI(not , E_NOT ) E_UNI(neg, E_NEG)

// binary
expr_t* e_jmp(expr_t* a)
{
	expr_t* ret = e_new();
	ret->type = E_JMP;
	ret->v.bin.a = a;
	ret->v.bin.b = NULL;
	return ret;
}

#define E_BIN(N, T) expr_t* e_##N(expr_t* a, expr_t* b) \
{ \
	expr_t* ret = e_new(); \
	ret->type = T; \
	ret->v.bin.a = a; \
	ret->v.bin.b = b; \
	return ret; \
}
E_BIN(jxx , E_JXX )
E_BIN(add , E_ADD ) E_BIN(sub, E_SUB) E_BIN(sbb, E_SBB) E_BIN(mul, E_MUL) E_BIN(div, E_DIV)
E_BIN(and , E_AND ) E_BIN(or , E_OR ) E_BIN(xor, E_XOR)
E_BIN(sar , E_SAR ) E_BIN(sal, E_SAL) E_BIN(shr, E_SHR) E_BIN(shl, E_SHL)
E_BIN(xchg, E_XCHG) E_BIN(mov, E_MOV) E_BIN(lea, E_LEA)

expr_t* e_test(expr_test_type_t t, expr_t* a)
{
	expr_t* ret = e_new();
	ret->type = E_TEST;
	ret->v.test.t = t;
	ret->v.test.a = a;
	return ret;
}
