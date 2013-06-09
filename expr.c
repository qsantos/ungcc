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
	ret->visited = false;
	return ret;
}

void e_del(expr_t* e)
{
	switch (e->type)
	{
	case E_OPERAND:
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
		e_del(e->v.uni.a);
		break;

	// binary
	case E_ADD:  case E_SUB: case E_SBB: case E_MUL: case E_DIV:
	case E_AND:  case E_OR:  case E_XOR:
	case E_SAR:  case E_SAL: case E_SHR: case E_SHL:
	case E_XCHG: case E_MOV: case E_LEA:
		e_del(e->v.bin.a);
		e_del(e->v.bin.b);
		break;
	}
	free(e);
}

expr_t* e_op_reg(reg_t reg)
{
	expr_t* ret = e_new();
	ret->type   = E_OPERAND;
	ret->v.op.t = REG;
	ret->v.op.v.reg  = reg;
	ret->v.op.symbol = NULL;
	return ret;
}

expr_t* e_op_im(im_t im)
{
	expr_t* ret = e_new();
	ret->type   = E_OPERAND;
	ret->v.op.t = IM;
	ret->v.op.v.im   = im;
	ret->v.op.symbol = NULL;
	return ret;
}

expr_t* e_op_addr(reg_t base, reg_t idx, im_t scale, im_t disp)
{
	expr_t* ret = e_new();
	ret->type   = E_OPERAND;
	ret->v.op.t = ADDR;
	ret->v.op.v.addr.base  = base;
	ret->v.op.v.addr.idx   = idx;
	ret->v.op.v.addr.scale = scale;
	ret->v.op.v.addr.disp  = disp;
	ret->v.op.symbol = NULL;
	return ret;
}

// zeroary
#define E_ZER(N, T) expr_t* e_##N() \
{ \
	expr_t* ret = e_new(); \
	ret->type = T; \
	return ret; \
}
E_ZER(nop  , E_NOP  )
E_ZER(ret  , E_RET  )
E_ZER(hlt  , E_HLT  )

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
	if (e == NULL || e->visited == false)
		return;
	e->visited = false;

	reset_visited(e->next);
	reset_visited(e->branch);
}

int cmp_op(operand_t* a, operand_t* b)
{
	if (a->t != b->t) return 1;

	if (a->t == REG)  return a->v.reg  == b->v.reg  ? 0 : 1;
	if (a->t == IM)   return a->v.im   == b->v.im   ? 0 : 1;
	if (a->t == ADDR) return memcmp(&a->v.addr, &b->v.addr, sizeof(addr_t));

	return 1;
}
