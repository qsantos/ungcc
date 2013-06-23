/*\
 *  This is an awesome programm simulating awesome battles of awesome robot tanks
 *  Copyright (C) 2013  Quentin SANTOS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/

#include "flist.h"

#include <stdlib.h>

void f_new(function_t* f, size_t address, expr_t* expr)
{
	f->name    = NULL;
	f->returns = true;
	f->argc    = 0;
	f->fast    = false;

	f->address = address;
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
	function_t key;
	key.address = address;
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
