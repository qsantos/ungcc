#include "block.h"

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
