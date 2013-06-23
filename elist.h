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

#ifndef ELIST_H
#define ELIST_H

#include <stdio.h>

#include "expr.h"

typedef struct
{
	size_t  o; // offset
	expr_t* e; // expression
} eopair_t;

typedef struct
{
	eopair_t* e;
	size_t    n;
	size_t    a;
} elist_t;

void      elist_new (elist_t* l);
void      elist_del (elist_t* l);
void      elist_push(elist_t* l, size_t o, expr_t* e);
eopair_t* elist_at  (elist_t* l, size_t o);

#endif
