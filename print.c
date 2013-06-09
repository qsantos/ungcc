#include "print.h"

#define PRTCHK(FCT, ...) {ret+=FCT(str+ret,size-ret,__VA_ARGS__);if(ret>=size)return ret;}

size_t print_reg(char* str, size_t size, reg_t reg, size_t s)
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

size_t print_hex(char* str, size_t size, im_t im)
{
	size_t ret = 0;

	if (im < 0)
		PRTCHK(snprintf, "-%#x", -im)
	else
		PRTCHK(snprintf, "%#x", im)

	return ret;
}

size_t print_op(char* str, size_t size, operand_t* op, size_t s)
{
	size_t ret = 0;

	if (op->symbol)
	{
		PRTCHK(snprintf, "%s", op->symbol);
		return ret;
	}

	if (op->t == REG)
		PRTCHK(print_reg, op->v.reg, s)
	else if (op->t == IM)
	{
		size_t v = op->v.im;
		if (0x1f < v && v < 0x7f)
			PRTCHK(snprintf, "'%c'", v)
		else if (v < 0x8048000)
			PRTCHK(snprintf, "%zi", v)
		else
		{
			PRTCHK(snprintf, "$");
			PRTCHK(print_hex, v);
		}
	}
	else if (op->t == ADDR)
	{
		addr_t* a = &op->v.addr;
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

	if (e->label)
		PRTCHK(snprintf, "<%s>:\n", e->label)

	if (e->type == E_OPERAND)
	{
		PRTCHK(print_op, &e->v.op, 32);
		return ret;
	}

	switch (e->type)
	{
	// zeroary
	PRINT_EXPR0(E_NOP, "nop");
	PRINT_EXPR0(E_RET, "ret");
	PRINT_EXPR0(E_HLT, "hlt");

	// unary
	PRINT_EXPR1(E_PUSH, "push"); PRINT_EXPR1(E_POP, "pop");
	PRINT_EXPR1(E_JMP,  "jmp");
	PRINT_EXPR1(E_JE,   "je");   PRINT_EXPR1(E_JNE, "jne");
	PRINT_EXPR1(E_JS,   "js");   PRINT_EXPR1(E_JNS, "jns");
	PRINT_EXPR1(E_JA,   "ja");   PRINT_EXPR1(E_JAE, "jae");
	PRINT_EXPR1(E_JB,   "jb");   PRINT_EXPR1(E_JBE, "jbe");
	PRINT_EXPR1(E_JL,   "jl");   PRINT_EXPR1(E_JLE, "jle");
	PRINT_EXPR1(E_JG,   "jg");   PRINT_EXPR1(E_JGE, "jge");
	PRINT_EXPR1(E_NOT,  "!");    PRINT_EXPR1(E_NEG, "~");

	case E_CALL:
		PRTCHK(print_expr, e->v.bin.a);
		PRTCHK(snprintf, "()");
		break;

	// binary
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
	default:
		PRTCHK(snprintf, "Unknown %zu\n", e->type);
	}

	return ret;
}

size_t print_stat(char* str, size_t size, expr_t* e)
{
	size_t ret = 0;
	PRTCHK(print_expr, e);
	PRTCHK(snprintf, ";\n");
	return ret;
}

void fprint_stat(FILE* f, expr_t* e)
{
	char buffer[1024];
	print_stat(buffer, 1024, e);
	fprintf(f, "%s", buffer);
}
