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

#ifndef PRINT_H
#define PRINT_H

#include <stdio.h>

#include "expr.h"

size_t print_reg (char* str, size_t size, expr_reg_type_t reg, size_t s);
size_t print_hex (char* str, size_t size, ssize_t v);
size_t print_expr(char* str, size_t size, expr_t* e);
size_t print_stat(char* str, size_t size, expr_t* i);
void  fprint_stat(FILE* f,   expr_t* e);

#endif
