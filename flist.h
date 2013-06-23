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

#ifndef FUNCTION_H
#define FUNCTION_H

typedef struct function function_t;

#include "expr.h"

struct function
{
	// 'fixed' information
	char*   name;
	bool    returns;
	size_t  argc;
	bool    fast; // fast parameter passing (eax)

	// 'variable' information
	size_t  address;
	expr_t* expr;
};

void f_new(function_t* f, size_t address, expr_t* expr);
void f_del(function_t* f);

typedef struct
{
	function_t* f;
	size_t      n;
	size_t      a;
} flist_t;

void flist_new(flist_t* l);
void flist_del(flist_t* l);

void        flist_sort(flist_t* l);
function_t* flist_find(flist_t* l, size_t address);
function_t* flist_push(flist_t* l, size_t address, expr_t* expr);

#endif
