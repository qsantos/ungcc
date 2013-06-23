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

#ifndef POSTPROC_H
#define POSTPROC_H

#include "elf.h"
#include "elist.h"
#include "flist.h"

// builds branching hierarchy and list functions into 'f'
void post_funs (flist_t* dst, elist_t* l, elf_t* elf);

void post_rmctx(expr_t* e); // strip context
void post_simpl(expr_t* e); // simplifications
void post_reduc(expr_t* e); // reductions

#endif
