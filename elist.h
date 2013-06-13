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
