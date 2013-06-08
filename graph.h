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
