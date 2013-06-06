#include "block.h"

#include <stdio.h>
#include <string.h>

#define PRTCHK(FCT, ...) {ret+=FCT(str+ret,size-ret,__VA_ARGS__);if(ret>=size)return ret;}

int cmp_op(operand_t* a, operand_t* b)
{
	if (a->t != b->t) return 1;

	if (a->t == REG)  return a->v.reg  == b->v.reg  ? 0 : 1;
	if (a->t == IM)   return a->v.im   == b->v.im   ? 0 : 1;
	if (a->t == ADDR) return memcmp(&a->v.addr, &b->v.addr, sizeof(addr_t));

	return 1;
}

size_t block_line(char* str, size_t size, instr_t* instr, size_t n_instr)
{
	if (n_instr == 0)
		return 0;

	opcode_t op = instr->op;

	if (op == O_NOP || op == O_JMP || op == O_CMP || op == O_TEST)
	{
//		*str = 0;
//		return 1;
	}

	if (op == O_XCHG && cmp_op(&instr->a, &instr->b) == 0)
	{
		*str = 0;
		return 1;
	}

	if (op == O_JE || op == O_JNE)
	{
		size_t ret = 0;
		for (; instr->op != O_CMP && instr->op != O_TEST; instr--);
		PRTCHK(snprintf, "if (");
		if (instr->op == O_CMP)
		{
			PRTCHK(print_op, &instr->b, instr->s);
			if (op == O_JE)
				PRTCHK(snprintf, " == ")
			else if (op == O_JNE)
				PRTCHK(snprintf, " != ")
			PRTCHK(print_op, &instr->a, instr->s);
		}
		else // O_TEST
		{
			PRTCHK(print_op, &instr->b, instr->s);
			if (cmp_op(&instr->a, &instr->b) != 0)
			{
				PRTCHK(snprintf, " && ");
				PRTCHK(print_op, &instr->a, instr->s);
			}
		}
		PRTCHK(snprintf, ")\n");
	}
	else
		print_instr(str, size, instr);
	return 1;
}

void blist_new(blist_t* l)
{
	l->b = NULL;
	l->n = 0;
	l->a = 0;
}

void blist_del(blist_t* l)
{
	free(l->b);
}

void blist_push(blist_t* l, instr_t* start, size_t size)
{
	if (l->n == l->a)
	{
		l->a = l->a ? 2*l->a : 1;
		l->b = (block_t*) realloc(l->b, l->a * sizeof(block_t));
	}

	block_t* b = l->b + l->n;
	b->start = start;
	b->size  = size;
	l->n++;
}

static int block_cmp(const void* a, const void* b)
{
	block_t* la = (block_t*) a;
	block_t* lb = (block_t*) b;

	size_t oa = la->start->offset;
	size_t ob = lb->start->offset;

	if (oa < ob) return -1;
	if (oa > ob) return  1;
	             return  0;
}

block_t* blist_search(blist_t* l, size_t offset)
{
	instr_t tmp;
	block_t key;
	tmp.offset = offset;
	key.start = &tmp;

	return bsearch(&key, l->b, l->n, sizeof(block_t), block_cmp);
}

void funs_new(functions_t* f)
{
	f->f = NULL;
	f->n = 0;
	f->a = 0;
}

void funs_del(functions_t* f)
{
	free(f->f);
}

void funs_push(functions_t* f, block_t* b)
{
	if (f->n == f->a)
	{
		f->a = f->a ? 2*f->a : 1;
		f->f = (block_t**) realloc(f->f, f->a * sizeof(block_t*));
	}

	f->f[f->n++] = b;
}
