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

static void print_reg(reg reg, size_t s)
{
	putchar('%');
	if (s == 8)
	{
		putchar('a' + ((reg-1)%4));
		if (reg < 4) putchar('h');
		else         putchar('l');
		return;
	}

	if (s == 64)
		printf("s");
	else if (s == 32)
		printf("e");
	
	if (reg <= 4)
	{
		putchar('a'-1 + reg);
		putchar('x');
	}
	else if (reg == 9)
		printf("sp");
	else if (reg == 10)
		printf("bp");
	else if (reg == 11)
		printf("si");
	else if (reg == 12)
		printf("di");
	else if (reg == 13)
		printf("zi");
	else
		printf("?");
}

static void print_hex(im im)
{
	if (im < 0)
		printf("-%#x", -im);
	else
		printf("%#x", im);
}

static void print_op(op* op, size_t s)
{
	if (op->symbol)
	{
		printf("%s", op->symbol);
		return;
	}

	if (op->t == REG)
		print_reg(op->v.reg, s);
	else if (op->t == IM)
	{
		putchar('$');
		print_hex(op->v.im);
	}
	else if (op->t == ADDR)
	{
		addr* a = &op->v.addr;
		if (a->disp)
			print_hex(a->disp);
		if (a->base)
		{
			putchar('(');
			print_reg(a->base, 32);
			if (a->scale)
			{
				putchar(',');
				print_reg(a->idx, 32);
				putchar(',');
				printf("%u", a->scale);
			}
			putchar(')');
		}
	}
}

#define PRINT_INSTR0(O,N) if(i->op==O){printf(N);}

#define PRINT_INSTR1(O,N) if(i->op==O){printf(N " ");\
	print_op(&i->a,i->s);}

#define PRINT_INSTR2(O,N) if(i->op==O){printf(N " ");\
	print_op(&i->a,i->s);\
	putchar(',');\
	print_op(&i->b,i->s);}

void instr_print(struct instr* i)
{
	if (i->label)
		printf("\n<%s>:\n", i->label);

	if (i->op == UNK)
		printf("=> %s", i->orig);
	else if (i->op == NOP)
		return;
//	else
//		printf("%-40s", i->orig);

//	PRINT_INSTR0(NOP,   "nop");
	PRINT_INSTR0(RET,   "ret");
	PRINT_INSTR0(LEAVE, "leave");
	PRINT_INSTR0(HLT,   "hlt");
	PRINT_INSTR1(PUSH,  "push");
	PRINT_INSTR1(POP,   "pop");
	PRINT_INSTR1(JMP,   "jmp");
	PRINT_INSTR1(JE,    "je");
	PRINT_INSTR1(JNE,   "jne");
	PRINT_INSTR1(JA,    "ja");
	PRINT_INSTR1(JB,    "jb");
	PRINT_INSTR1(JS,    "js");
	PRINT_INSTR1(JNS,   "jns");
	PRINT_INSTR1(JL,    "jl");
	PRINT_INSTR1(JLE,   "jle");
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

	if (i->op == MOV)
	{
		print_op(&i->b, i->s);
		printf(" = ");
		print_op(&i->a, i->s);
	}
	if (i->op == LEA)
	{
		print_op(&i->b, i->s);
		printf(" = &");
		print_op(&i->a, i->s);
	}
	printf("\n");
}

void asm_print(struct asm* asm)
{
	for (size_t k = 0; k < asm->n; k++)
		instr_print(asm->i + k);

}

int cmp_offset(const void* a, const void* b)
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
