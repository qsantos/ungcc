#include "print.h"

#define PRTCHK(FCT, ...) {ret+=FCT(str+ret,size-ret,__VA_ARGS__);if(ret>=size)return ret;}

size_t print_reg(char* str, size_t size, rtype_t reg, size_t s)
{
	size_t ret = 0;

	PRTCHK(snprintf, "%%");

	if (s == 64)
		PRTCHK(snprintf, "s")
	else if (s == 32)
		PRTCHK(snprintf, "e")

	if      (reg == R_IZ) PRTCHK(snprintf, "zi")
	else if (reg == R_AX) PRTCHK(snprintf, "ax")
	else if (reg == R_BX) PRTCHK(snprintf, "bx")
	else if (reg == R_CX) PRTCHK(snprintf, "cx")
	else if (reg == R_DX) PRTCHK(snprintf, "dx")
	else if (reg == R_AL) PRTCHK(snprintf, "al")
	else if (reg == R_BL) PRTCHK(snprintf, "bl")
	else if (reg == R_CL) PRTCHK(snprintf, "cl")
	else if (reg == R_DL) PRTCHK(snprintf, "dl")
	else if (reg == R_AH) PRTCHK(snprintf, "ah")
	else if (reg == R_BH) PRTCHK(snprintf, "bh")
	else if (reg == R_CH) PRTCHK(snprintf, "ch")
	else if (reg == R_DH) PRTCHK(snprintf, "dh")
	else if (reg == R_SP) PRTCHK(snprintf, "sp")
	else if (reg == R_BP) PRTCHK(snprintf, "bp")
	else if (reg == R_SI) PRTCHK(snprintf, "si")
	else if (reg == R_DI) PRTCHK(snprintf, "di")
	else if (reg == R_FL) PRTCHK(snprintf, "fl")
	else if (R_ST0 <= reg && reg <= R_ST1) PRTCHK(snprintf, "st(%zu)", reg - R_ST0)
	else                  PRTCHK(snprintf, "?")

	return ret;
}

size_t print_hex(char* str, size_t size, ssize_t v)
{
	size_t ret = 0;

	if (v < 0)
		PRTCHK(snprintf, "-%#x", -v)
	else
		PRTCHK(snprintf, "%#x", v)

	return ret;
}

#define PRINT_EXPR0(O,N) case O: \
	PRTCHK(snprintf, N); \
	break;

#define PRINT_EXPR1(O,N) case O: \
	PRTCHK(snprintf, N " "); \
	PRTCHK(print_expr, e->v.bin.a); \
	break;

#define PRINT_EXPR2(O,N) case O: \
	PRTCHK(print_expr, e->v.bin.a); \
	PRTCHK(snprintf, " " N " "); \
	PRTCHK(print_expr, e->v.bin.b); \
	break;

size_t print_expr(char* str, size_t size, expr_t* e)
{
	size_t ret = 0;
	*str = 0;

	if (e->used == 1)
		return ret;

	switch (e->type)
	{
	case E_UNK:
		PRTCHK(snprintf, "Unsupported '%s'", e->v.unk);
		break;

	case E_REG:
	{
		expr_t* l = e->v.reg.last;
		if (l && l->used == 1)
			PRTCHK(print_expr, l->v.bin.b)
		else
			PRTCHK(print_reg, e->v.reg.t, 32)
		break;
	}

	case E_IM:
	{
		size_t v = e->v.im.v;
		if (e->v.im.sym && e->v.im.sym->name)
			PRTCHK(snprintf, "%s", e->v.im.sym->name)
		else if (e->v.im.str)
			PRTCHK(snprintf, "\"%s\"", e->v.im.str)
		else if (0x1f < v && v < 0x7f)
			PRTCHK(snprintf, "'%c'", v)
		else if (v < 0x8048000)
			PRTCHK(snprintf, "%zi", v)
		else
		{
			PRTCHK(snprintf, "$");
			PRTCHK(print_hex, v);
		}
		break;
	}

	case E_ADDR:
	{
		addr_t* a = &e->v.addr;
/*
For reminder, the context looks like:

+
      .
      .          parameters of the current functions
      .
    param0
 -------------
      RA
 -------------
   prev. BP
 -----BP------
    local0
      .
      .          local variables
      .
    localN
      ?
    paramM
      .
      .          paramters for the called function
      .
    param0
 -----SP------
-

*/
		if (a->base == R_BP)
		{
			if (a->disp >= 0) // parameters
				PRTCHK(snprintf, "param%zu", a->disp - 8) // RA and previous BP pushed on stack
			else // local variable
				PRTCHK(snprintf, "local%zu", -a->disp)
			if (a->scale)
			{
				PRTCHK(snprintf, "[");
				PRTCHK(print_reg, a->idx, 32);
				PRTCHK(snprintf, "]_%zu", a->scale);
			}
		}
		else if (a->base == R_SP) // parameters for called function
		{
			PRTCHK(snprintf, "nparam%zu", a->disp)
			if (a->scale)
			{
				PRTCHK(snprintf, "[");
				PRTCHK(print_reg, a->idx, 32);
				PRTCHK(snprintf, "]_%zu", a->scale);
			}
		}
		else
		{
			if (a->disp)
				PRTCHK(print_hex, a->disp);
			if (a->base)
			{
				PRTCHK(snprintf, "(");
				PRTCHK(print_reg, a->base, 32);
				if (a->scale)
				{
					PRTCHK(snprintf, ",");
					PRTCHK(print_reg, a->idx, 32);
					PRTCHK(snprintf, ", %u", a->scale);
				}
				PRTCHK(snprintf, ")");
			}
		}
	}

	case E_NOP:
		// ignore
		break;

	// zeroary
	PRINT_EXPR0(E_RET, "ret");
	PRINT_EXPR0(E_HLT, "hlt");

	// unary
	PRINT_EXPR1(E_PUSH, "push"); PRINT_EXPR1(E_POP, "pop");
	PRINT_EXPR1(E_NOT,  "!");    PRINT_EXPR1(E_NEG, "~");

	case E_CALL:
		PRTCHK(print_expr, e->v.bin.a);
		PRTCHK(snprintf, "()");
		break;

	// binary
	case E_JXX:
		PRTCHK(snprintf, "if (");
		PRTCHK(print_expr, e->v.bin.b);
		PRTCHK(snprintf, ")");
		break;

	case E_JMP:
	{
		expr_t* a = e->v.bin.a;
		if (a->type == E_IM && a->v.im.sym)
			PRTCHK(snprintf, "-> %s()", a->v.im.sym->name)
		break;
	}

	PRINT_EXPR2(E_ADD,  "+");
	PRINT_EXPR2(E_SUB,  "-");
	PRINT_EXPR2(E_SBB,  "-'"); // TODO
	PRINT_EXPR2(E_MUL,  "*");
	PRINT_EXPR2(E_DIV,  "/");
	PRINT_EXPR2(E_AND,  "&");
	PRINT_EXPR2(E_OR,   "|");
	PRINT_EXPR2(E_XOR,  "^");
	PRINT_EXPR2(E_SAR,  ">>");
	PRINT_EXPR2(E_SAL,  "<<");
	PRINT_EXPR2(E_SHR,  ">>'"); // TODO
	PRINT_EXPR2(E_SHL,  "<<'"); // TODO
	PRINT_EXPR2(E_XCHG, "↔");
	PRINT_EXPR2(E_MOV,  "=");
	PRINT_EXPR2(E_LEA,  "=&");

	case E_TEST:
		PRTCHK(print_expr, e->v.test.a);
		switch (e->v.test.t)
		{
		case T_E:  PRTCHK(snprintf, " == 0");  break;
		case T_NE: PRTCHK(snprintf, " != 0");  break;
		case T_S:  PRTCHK(snprintf, " <=_ 0"); break;
		case T_NS: PRTCHK(snprintf, " >=_ 0"); break;
		case T_A:  PRTCHK(snprintf, " > 0");   break;
		case T_AE: PRTCHK(snprintf, " >= 0");  break;
		case T_B:  PRTCHK(snprintf, " < 0");   break;
		case T_BE: PRTCHK(snprintf, " <= 0");  break;
		}
		break;

	default:
		PRTCHK(snprintf, "Unknown %zu\n", e->type);
	}

	return ret;
}

size_t print_stat(char* str, size_t size, expr_t* e)
{
	size_t ret = 0;

	if (e->label)
		PRTCHK(snprintf, "<%s>:\n", e->label)

	size_t start = ret;
	PRTCHK(print_expr, e);
	if (ret != start)
		PRTCHK(snprintf, "\n");

	return ret;
}

void fprint_stat(FILE* f, expr_t* e)
{
	char buffer[1024];
	print_stat(buffer, 1024, e);
	fprintf(f, "%s", buffer);
}
