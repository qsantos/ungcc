#include "block.h"

#include <stdio.h>
#include <string.h>

#define PRTCHK(FCT, ...) {ret+=FCT(str+ret,size-ret,__VA_ARGS__);if(ret>=size)return ret;}

int cmp_op(op* a, op* b)
{
	if (a->t != b->t) return 1;

	if (a->t == REG)  return a->v.reg  == b->v.reg  ? 0 : 1;
	if (a->t == IM)   return a->v.im   == b->v.im   ? 0 : 1;
	if (a->t == ADDR) return memcmp(&a->v.addr, &b->v.addr, sizeof(addr));

	return 1;
}

size_t block_line(char* str, size_t size, struct instr* instr, size_t n_instr)
{
	if (n_instr == 0)
		return 0;

	enum opcode op = instr->op;

	if (op == NOP || op == JMP || op == CMP || op == TEST)
	{
		*str = 0;
		return 1;
	}

	if (op == XCHG && cmp_op(&instr->a, &instr->b) == 0)
	{
		*str = 0;
		return 1;
	}

	if (op == JE || op == JNE)
	{
		size_t ret = 0;
		for (; instr->op != CMP && instr->op != TEST; instr--);
		PRTCHK(snprintf, "if (");
		if (instr->op == CMP)
		{
			PRTCHK(print_op, &instr->b, instr->s);
			if (op == JE)
				PRTCHK(snprintf, " == ")
			else if (op == JNE)
				PRTCHK(snprintf, " != ")
			PRTCHK(print_op, &instr->a, instr->s);
		}
		else // TEST
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

void blist_new(struct blist* l)
{
	l->b = NULL;
	l->n = 0;
	l->a = 0;
}

void blist_del(struct blist* l)
{
	free(l->b);
}

void blist_push(struct blist* l, struct instr* start, size_t size)
{
	if (l->n == l->a)
	{
		l->a = l->a ? 2*l->a : 1;
		l->b = (struct block*) realloc(l->b, l->a * sizeof(struct block));
	}

	struct block* b = l->b + l->n;
	b->start = start;
	b->size  = size;
	l->n++;
}

static int block_cmp(const void* a, const void* b)
{
	struct block* la = (struct block*) a;
	struct block* lb = (struct block*) b;

	size_t oa = la->start->offset;
	size_t ob = lb->start->offset;

	if (oa < ob) return -1;
	if (oa > ob) return  1;
	             return  0;
}

struct block* blist_search(struct blist* l, size_t offset)
{
	struct instr tmp;
	struct block key;
	tmp.offset = offset;
	key.start = &tmp;

	return bsearch(&key, l->b, l->n, sizeof(struct block), block_cmp);
}

void funs_new(struct functions* f)
{
	f->f = NULL;
	f->n = 0;
	f->a = 0;
}

void funs_del(struct functions* f)
{
	free(f->f);
}

void funs_push(struct functions* f, struct block* b)
{
	if (f->n == f->a)
	{
		f->a = f->a ? 2*f->a : 1;
		f->f = (struct block**) realloc(f->f, f->a * sizeof(struct block*));
	}

	f->f[f->n++] = b;
}
