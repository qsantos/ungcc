#ifndef POSTPROC_H
#define POSTPROC_H

#include "elist.h"

void post_funs (elist_t* dst, elist_t* l, size_t entryPoint);
void post_rmctx(expr_t* e);
void post_simpl(expr_t* e);
void post_reduc(expr_t* e);

#endif
