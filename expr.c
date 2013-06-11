#include "expr.h"

#include <stdlib.h>
#include <string.h>

expr_t* e_new()
{
	expr_t* ret = malloc(sizeof(expr_t));
	ret->label   = NULL;
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
	switch (e->type)
	{
	case E_UNK:
		free(e->v.unk);
		break;

	case E_REG:
		break;
	case E_IM:
		free(e->v.im.symbol);
		break;
	case E_ADDR:
		break;

	// zeroary
	case E_NOP:
	case E_RET:
	case E_HLT:
		break;

	// unary
	case E_PUSH: case E_POP:
	case E_JMP:
	case E_JE:   case E_JNE:
	case E_JS:   case E_JNS:
	case E_JA:   case E_JAE:
	case E_JB:   case E_JBE:
	case E_JL:   case E_JLE:
	case E_JG:   case E_JGE:
	case E_CALL:
	case E_NOT:  case E_NEG:
		e_del(e->v.uni.a, false);
		break;

	// binary
	case E_ADD:  case E_SUB: case E_SBB: case E_MUL: case E_DIV:
	case E_AND:  case E_OR:  case E_XOR:
	case E_SAR:  case E_SAL: case E_SHR: case E_SHL:
	case E_XCHG: case E_MOV: case E_LEA:
		e_del(e->v.bin.a, false);
		e_del(e->v.bin.b, false);
		break;
	}
	
	if (!keep)
		free(e);
}

expr_t* e_cpy(expr_t* e)
{
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
		ret->v.im.v = e->v.im.v;
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

	// unary
	case E_PUSH: case E_POP:
	case E_JMP:
	case E_JE:   case E_JNE:
	case E_JS:   case E_JNS:
	case E_JA:   case E_JAE:
	case E_JB:   case E_JBE:
	case E_JL:   case E_JLE:
	case E_JG:   case E_JGE:
	case E_CALL:
	case E_NOT:  case E_NEG:
		ret->v.uni.a = e_cpy(e->v.uni.a);
		break;

	// binary
	case E_ADD:  case E_SUB: case E_SBB: case E_MUL: case E_DIV:
	case E_AND:  case E_OR:  case E_XOR:
	case E_SAR:  case E_SAL: case E_SHR: case E_SHL:
	case E_XCHG: case E_MOV: case E_LEA:
		ret->v.bin.a = e_cpy(e->v.bin.a);
		ret->v.bin.b = e_cpy(e->v.bin.b);
		break;
	}
	return ret;
}

expr_t* e_unk(char* comment)
{
	expr_t* ret = e_new();
	ret->type = E_UNK;
	ret->v.unk = comment;
	return ret;
}

// register
expr_t* e_reg(rtype_t reg)
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
	ret->type        = E_IM;
	ret->v.im.v      = im;
	ret->v.im.symbol = NULL;
	return ret;
}

// address
expr_t* e_addr(rtype_t base, rtype_t idx, size_t scale, ssize_t disp)
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

// unary
#define E_UNI(N, T) expr_t* e_##N(expr_t* a) \
{ \
	expr_t* ret = e_new(); \
	ret->type = T; \
	ret->v.uni.a = a; \
	return ret; \
}
E_UNI(push, E_PUSH) E_UNI(pop, E_POP)
E_UNI(jmp , E_JMP )
E_UNI(je  , E_JE  ) E_UNI(jne, E_JNE)
E_UNI(js  , E_JS  ) E_UNI(jns, E_JNS)
E_UNI(ja  , E_JA  ) E_UNI(jae, E_JAE)
E_UNI(jb  , E_JB  ) E_UNI(jbe, E_JBE)
E_UNI(jl  , E_JL  ) E_UNI(jle, E_JLE)
E_UNI(jg  , E_JG  ) E_UNI(jge, E_JGE)
E_UNI(call, E_CALL)
E_UNI(not , E_NOT ) E_UNI(neg, E_NEG)

// binary
#define E_BIN(N, T) expr_t* e_##N(expr_t* a, expr_t* b) \
{ \
	expr_t* ret = e_new(); \
	ret->type = T; \
	ret->v.bin.a = a; \
	ret->v.bin.b = b; \
	return ret; \
}
E_BIN(add , E_ADD ) E_BIN(sub, E_SUB) E_BIN(sbb, E_SBB) E_BIN(mul, E_MUL) E_BIN(div, E_DIV)
E_BIN(and , E_AND ) E_BIN(or , E_OR ) E_BIN(xor, E_XOR)
E_BIN(sar , E_SAR ) E_BIN(sal, E_SAL) E_BIN(shr, E_SHR) E_BIN(shl, E_SHL)
E_BIN(xchg, E_XCHG) E_BIN(mov, E_MOV) E_BIN(lea, E_LEA)

void reset_visited(expr_t* e)
{
	if (e == NULL || !e->visited)
		return;
	e->visited = false;

	reset_visited(e->next);
	reset_visited(e->branch);
}

#define CMP1(T) case T: return cmp_expr(a->v.uni.a, b->v.uni.a);
#define CMP2(T) case T: return cmp_expr(a->v.bin.a, b->v.bin.a) || cmp_expr(a->v.bin.b, b->v.bin.b);

int cmp_expr(expr_t* a, expr_t* b)
{
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
	// unary
	CMP1(E_PUSH); CMP1(E_POP);
	CMP1(E_JMP);
	CMP1(E_JE);   CMP1(E_JNE);
	CMP1(E_JS);   CMP1(E_JNS);
	CMP1(E_JA);   CMP1(E_JAE);
	CMP1(E_JB);   CMP1(E_JBE);
	CMP1(E_JL);   CMP1(E_JLE);
	CMP1(E_JG);   CMP1(E_JGE);
	CMP1(E_CALL);
	CMP1(E_NOT);  CMP1(E_NEG);

	// binary
	CMP2(E_ADD);  CMP2(E_SUB); CMP2(E_SBB); CMP2(E_MUL); CMP2(E_DIV);
	CMP2(E_AND);  CMP2(E_OR);  CMP2(E_XOR);
	CMP2(E_SAR);  CMP2(E_SAL); CMP2(E_SHR); CMP2(E_SHL);
	CMP2(E_XCHG); CMP2(E_MOV); CMP2(E_LEA);
	}
	return 1;
}
