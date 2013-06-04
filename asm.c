#include "asm.h"

#include <stdio.h>

void asm_new(struct asm* asm)
{
	asm->i = NULL;
	asm->a = 0;
	asm->n = 0;
}

void asm_del(struct asm* asm)
{
	for (size_t i = 0; i < asm->n; i++)
	{
		free(asm->i[i].label);
		free(asm->i[i].orig);
	}
	free(asm->i);
	free(asm);
}

struct instr* asm_next(struct asm* asm, size_t offset, char* orig, char* label)
{
	if (asm->n == asm->a)
	{
		asm->a = asm->a ? 2*asm->a : 1;
		asm->i = (struct instr*) realloc(asm->i, asm->a * sizeof(struct instr));
	}

	struct instr* i = &asm->i[asm->n++];
	i->offset   = offset;
	i->orig     = orig;
	i->label    = label;
	i->function = false;
	i->branch   = false;
	return i;
}

void asm_set_reg(op* op, reg reg)
{
	op->t = REG;
	op->v.reg = reg;
	op->symbol = NULL;
}

void asm_set_im(op* op, im im)
{
	op->t = IM;
	op->v.im = im;
	op->symbol = NULL;
}

void asm_set_addr(op* op, reg base, reg idx, im scale, im disp)
{
	op->t = ADDR;
	op->v.addr.base  = base;
	op->v.addr.idx   = idx;
	op->v.addr.scale = scale;
	op->v.addr.disp  = disp;
	op->symbol = NULL;
}

#define PRTCHK(FCT, ...) {ret+=FCT(str+ret,size-ret,__VA_ARGS__);if(ret>=(int)size)return ret;}

static int print_reg(char* str, size_t size, reg reg, size_t s)
{
	int ret = 0;

	PRTCHK(snprintf, "%%");
	if (s == 8)
	{
		PRTCHK(snprintf, "%c%c", 'a' + ((reg-1)%4), reg < 4 ? 'h' : 'l');
		return ret;
	}

	if (s == 64)
		PRTCHK(snprintf, "s")
	else if (s == 32)
		PRTCHK(snprintf, "e")
	
	if (reg <= 4)
		PRTCHK(snprintf, "%cx", 'a'-1 + reg)
	else if (reg == 9)
		PRTCHK(snprintf, "sp")
	else if (reg == 10)
		PRTCHK(snprintf, "bp")
	else if (reg == 11)
		PRTCHK(snprintf, "si")
	else if (reg == 12)
		PRTCHK(snprintf, "di")
	else if (reg == 13)
		PRTCHK(snprintf, "zi")
	else
		PRTCHK(snprintf, "?")

	return ret;
}

static int print_hex(char* str, size_t size, im im)
{
	int ret = 0;

	if (im < 0)
		PRTCHK(snprintf, "-%#x", -im)
	else
		PRTCHK(snprintf, "%#x", im)
	
	return ret;
}

static int print_op(char* str, size_t size, op* op, size_t s)
{
	int ret = 0;

	if (op->symbol)
	{
		PRTCHK(snprintf, "%s", op->symbol);
		return ret;
	}

	if (op->t == REG)
		PRTCHK(print_reg, op->v.reg, s)
	else if (op->t == IM)
	{
		PRTCHK(snprintf, "$");
		PRTCHK(print_hex, op->v.im);
	}
	else if (op->t == ADDR)
	{
		addr* a = &op->v.addr;
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

#define PRINT_INSTR0(O,N) if(i->op==O){PRTCHK(snprintf, N);}

#define PRINT_INSTR1(O,N) if(i->op==O){PRTCHK(snprintf, N " ");\
	PRTCHK(print_op, &i->a,i->s);}

#define PRINT_INSTR2(O,N) if(i->op==O){PRTCHK(snprintf, N " ");\
	PRTCHK(print_op, &i->a,i->s);\
	PRTCHK(snprintf, ",");\
	PRTCHK(print_op, &i->b,i->s);}

int snprint_instr(char* str, size_t size, struct instr* i)
{
	int ret = 0;
	*str = 0;

	if (i->label)
		PRTCHK(snprintf, "<%s>:\n", i->label)

	if (i->op == UNK)
		PRTCHK(snprintf, "=> %s", i->orig)

//	PRINT_INSTR0(NOP,   "nop");
	PRINT_INSTR0(RET,   "ret");
	PRINT_INSTR0(LEAVE, "leave");
	PRINT_INSTR0(HLT,   "hlt");
	PRINT_INSTR1(PUSH,  "push");
	PRINT_INSTR1(POP,   "pop");
//	PRINT_INSTR1(JMP,   "jmp");
	PRINT_INSTR1(JE,    "je");
	PRINT_INSTR1(JNE,   "jne");
	PRINT_INSTR1(JA,    "ja");
	PRINT_INSTR1(JAE,   "jae");
	PRINT_INSTR1(JB,    "jb");
	PRINT_INSTR1(JBE,   "jbe");
	PRINT_INSTR1(JS,    "js");
	PRINT_INSTR1(JNS,   "jns");
	PRINT_INSTR1(JL,    "jl");
	PRINT_INSTR1(JLE,   "jle");
	PRINT_INSTR1(JG,    "jg");
	PRINT_INSTR1(JGE,   "jge");
	PRINT_INSTR1(CALL,  "call");
	PRINT_INSTR1(NOT,   "not");
	PRINT_INSTR1(NEG,   "neg");
	PRINT_INSTR2(XCHG,  "xchg");
	PRINT_INSTR2(ADD,   "add");
	PRINT_INSTR2(SUB,   "sub");
	PRINT_INSTR2(MUL,   "mul");
	PRINT_INSTR2(DIV,   "div");
	PRINT_INSTR2(AND,   "and");
	PRINT_INSTR2(OR,    "or");
	PRINT_INSTR2(XOR,   "xor");
	PRINT_INSTR2(SAR,   "sar");
	PRINT_INSTR2(SAL,   "sal");
	PRINT_INSTR2(SHR,   "shr");
	PRINT_INSTR2(SHL,   "shl");
	PRINT_INSTR2(TEST,  "test");
	PRINT_INSTR2(CMP,   "cmp");
	PRINT_INSTR2(MOV,   "mov");
	PRINT_INSTR2(LEA,   "lea");

	PRTCHK(snprintf, "\n");

	return ret;
}

void print_instr(struct instr* i)
{
	char buffer[1024];
	snprint_instr(buffer, 1024, i);
	printf("%s", buffer);
}

static int cmp_offset(const void* a, const void* b)
{
	struct instr* ia = (struct instr*) a;
	struct instr* ib = (struct instr*) b;

	if (ia->offset < ib->offset) return -1;
	if (ia->offset > ib->offset) return  1;
	return 0;
}

struct instr* offset2instr(struct asm* asm, size_t offset)
{
	struct instr key;
	key.offset = offset;
	return bsearch(&key, asm->i, asm->n, sizeof(struct instr), cmp_offset);
}
