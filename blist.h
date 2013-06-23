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

#ifndef GRAPH_H
#define GRAPH_H

#include "expr.h"

// recursive structure
typedef struct block block_t;

struct block
{
	expr_t*  e;
	block_t* next;
	block_t* branch;

	double x;
	double y;
	double h;
	double w;

	bool visited;
};

typedef struct
{
	block_t* b;
	size_t   n;
	size_t   a;
} blist_t;

void blist_new   (blist_t* l);
void blist_del   (blist_t* l);
void blist_push  (blist_t* l, expr_t* e);
void blist_gen   (blist_t* l, expr_t* e);
void blist_spread(blist_t* l);
void blist_reset (blist_t* l);

#endif
