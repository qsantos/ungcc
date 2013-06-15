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
