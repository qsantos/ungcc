#include "code.h"

#include <stdio.h>

void code_new(struct code* c)
{
	c->i = NULL;
	c->a = 0;
	c->n = 0;
}

void code_del(struct code* c)
{
	for (size_t i = 0; i < c->n; i++)
		free(c->i[i].orig);
	free(c->i);
	free(c);
}

struct instr* code_next(struct code* c, size_t offset, char* orig)
{
	if (c->n == c->a)
	{
		c->a = c->a ? 2*c->a : 1;
		c->i = (struct instr*) realloc(c->i, c->a * sizeof(struct instr));
	}

	struct instr* i = &c->i[c->n++];
	i->offset = offset;
	i->orig   = orig;
	return i;
}

void code_set_reg(op* op, reg reg)
{
	op->t = REG;
	op->v.reg = reg;
}

void code_set_im(op* op, im im)
{
	op->t = IM;
	op->v.im = im;
}

void code_set_addr(op* op, reg base, reg idx, im scale, im disp)
{
	op->t = ADDR;
	op->v.addr.base  = base;
	op->v.addr.idx   = idx;
	op->v.addr.scale = scale;
	op->v.addr.disp  = disp;
}

static void print_reg(reg reg, size_t s)
{
	putchar('%');
	if (s == 8)
	{
		putchar('a' + (reg%4));
		if (reg < 4) putchar('h');
		else         putchar('l');
		return;
	}

	if (s == 64)
		printf("s");
	else if (s == 32)
		printf("e");
	
	if (reg < 4)
	{
		putchar('a' + reg);
		putchar('x');
	}
	else if (reg == 8)
		printf("sp");
	else if (reg == 9)
		printf("bp");
	else if (reg == 10)
		printf("si");
	else if (reg == 11)
		printf("di");
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

void code_print(struct code* c)
{
	for (size_t k = 0; k < c->n; k++)
	{
		struct instr* i = &c->i[k];
		if (!i->op) continue;

		printf("%-30s → %zu ", i->orig, i->op);
		print_op(&i->a, i->s);
		printf(", ");
		print_op(&i->b, i->s);
		printf("\n");
	}
}
