#include "function.h"

#include <stdlib.h>

void f_new(function_t* f, size_t address, expr_t* expr)
{
	f->address = address;
	f->name    = NULL;
	f->expr    = expr;
}

void f_del(function_t* f)
{
//	e_del(f->expr, false); // TODO
	free(f->name);
}

void flist_new(flist_t* l)
{
	l->f = NULL;
	l->n = 0;
	l->a = 0;
}

void flist_del(flist_t* l)
{
	for (size_t i = 0; i < l->n; i++)
		f_del(l->f+i);
	free(l->f);
}

static int f_cmp(const void* a, const void* b)
{
	const function_t* fa = (const function_t*) a;
	const function_t* fb = (const function_t*) b;

	if (fa->address < fb->address) return -1;
	if (fa->address > fb->address) return  1;
	                               return  0;
}

void flist_sort(flist_t* l)
{
	qsort(l->f, l->n, sizeof(function_t), f_cmp);
}

function_t* flist_find(flist_t* l, size_t address)
{
	function_t key = {address, NULL, NULL};
	return bsearch(&key, l->f, l->n, sizeof(function_t), f_cmp);
}

function_t* flist_push(flist_t* l, size_t address, expr_t* expr)
{
	if (l->n == l->a)
	{
		l->a = l->a ? 2*l->a : 1;
		l->f = (function_t*) realloc(l->f, l->a * sizeof(function_t));
	}
	function_t* f = &l->f[l->n++];
	f_new(f, address, expr);
	return f;
}
