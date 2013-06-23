/*\
 *  Binary analyzer for decompilation and forensics
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

#include "blist.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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

void blist_push(blist_t* l, expr_t* e)
{
	if (l->n == l->a)
	{
		l->a = l->a ? 2*l->a : 1;
		l->b = (block_t*) realloc(l->b, l->a * sizeof(block_t));
	}
	block_t* ret = &l->b[l->n++];
	ret->e       = e;
	ret->next    = 0;
	ret->branch  = 0;
	ret->x       = 0;
	ret->y       = 0;
	ret->h       = 0;
	ret->w       = 0;
	ret->visited = false;
}

static void blist_gen_aux1(blist_t* l, expr_t* e)
{
	if (e == NULL || e->visited)
		return;
	e->visited = true;

	// create new block
	blist_push(l, e);

	// goes to end of block
	for (; !e->endBlck; e = e->next, e->visited = true);

	// induction
	blist_gen_aux1(l, e->next);
	blist_gen_aux1(l, e->branch);
}
static int cmp_block(const void* a, const void* b)
{
	const block_t* ba = (const block_t*) a;
	const block_t* bb = (const block_t*) b;

	if (ba->e < bb->e) return -1;
	if (ba->e > bb->e) return  1;
	                   return  0;
}
static void blist_gen_aux2(blist_t* l, expr_t* e)
{
	if (e == NULL || e->visited)
		return;
	e->visited = true;

	// gets current block
	block_t key;
	key.e = e;
	block_t* b = bsearch(&key, l->b, l->n, sizeof(block_t), cmp_block);
	if (b == NULL)
	{
		fprintf(stderr, "Unexpected error when detecting blocks\n");
		exit(1);
	}

	// goes to end of block
	for (; !e->endBlck; e = e->next, e->visited = true);

	// handle next and branch blocks
	if (e->next)
	{
		key.e = e->next;
		block_t* c = bsearch(&key, l->b, l->n, sizeof(block_t), cmp_block);
		b->next = c;
		blist_gen_aux2(l, e->next); // induction
	}
	if (e->branch)
	{
		key.e = e->branch;
		block_t* c = bsearch(&key, l->b, l->n, sizeof(block_t), cmp_block);
		b->branch = c;
		blist_gen_aux2(l, e->branch); // induction
	}
}
void blist_gen(blist_t* l, expr_t* e)
{
	// create blocks
	e_rstvisited(e);
	blist_new(l);
	blist_gen_aux1(l, e);

	// sort blocks by expression address
	qsort(l->b, l->n, sizeof(block_t), cmp_block);

	// create hierarchy
	e_rstvisited(e);
	blist_gen_aux2(l, e);
}

#define ATTRACT 0.01
#define PUSH    10000000
static void attract(double* x, double* y, const block_t* a, const block_t* b, double f)
{
	double dx = b->x - a->x;
	double dy = b->y - (a->y + a->h + 200);
	dx *= 10;
	dy *= 1 + 4*(dy < 0);

	*x -= dx * ATTRACT * f;
	*y -= dy * ATTRACT * f;
}

static void pushAway(double* x, double* y, const block_t* a, const block_t* b)
{
	double dx = b->x - a->x;
	double dy = b->y - a->y;
	double d2 = dx*dx + dy*dy;

	if (d2 == 0)
		return;

	double factor = 1/sqrt(d2) * PUSH / d2;
	*x -= dx * factor;
	(void) y;
//	*y -= dy * factor;
}
void blist_spread(blist_t* l)
{
	double cury = 0;
	for (size_t i = 0; i < l->n; i++)
	{
		l->b[i].x = rand() % 10000;
		l->b[i].y = cury;
		cury += l->b[i].h + 100;
	}
	for (size_t bla = 0; bla < 100; bla++)
	{
		for (size_t i = 0; i < l->n; i++)
		{
			block_t* b = l->b+i;
			double dx = 0;
			double dy = 0;// 2* ( (double)k - funlen/2 ); // verticality

			for (size_t i = 0; i < l->n; i++)
			{
				block_t* c = l->b+i;
				if (c == b->next || c == b->branch); // b is parent
				else if (c->next   == b) attract(&dx, &dy, c, b, 10);
				else if (c->branch == b) attract(&dx, &dy, c, b, 1);
				else                     pushAway(&dx, &dy, b, c);
			}

//			dx /= b->size;
//			dy /= b->size;
			b->x += dx;
			b->y += dy;
		}
	}
}

void blist_reset(blist_t* l)
{
	for (size_t i = 0; i < l->n; i++)
		l->b[i].visited = false;
}
