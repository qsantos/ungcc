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

static int eopair_cmp(const void* a, const void* b)
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
	return bsearch(&key, l->e, l->n, sizeof(eopair_t), eopair_cmp);
}
