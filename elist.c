#include "elist.h"

#include <stdlib.h>

void elist_new(elist_t* l)
{
	l->e = NULL;
	l->n = 0;
	l->a = 0;
}

void elist_del(elist_t* l)
{
	// TODO
	free(l->e);
}

void elist_push(elist_t* l, size_t o, expr_t* e)
{
	if (l->n == l->a)
	{
		l->a = l->a ? 2*l->a : 1;
		l->e = (eopair_t*) realloc(l->e, l->a * sizeof(eopair_t));
	}
	l->e[l->n].o = o;
	l->e[l->n].e = e;
	l->n++;
}

static int cmp_eopair(const void* a, const void* b)
{
	const eopair_t* eoa = (const eopair_t*) a;
	const eopair_t* eob = (const eopair_t*) b;

	if (eoa->o < eob->o) return -1;
	if (eoa->o > eob->o) return  1;
	                     return  0;
}
eopair_t* elist_at(elist_t* l, size_t o)
{
	eopair_t key = {o, NULL};
	return bsearch(&key, l->e, l->n, sizeof(eopair_t), cmp_eopair);
}
