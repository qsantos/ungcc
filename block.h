#ifndef BLOCK_H
#define BLOCK_H

#include "asm.h"

// recursive structure
typedef struct block block_t;

struct block
{
	// block information
	instr_t* start;
	size_t   size;

	// branching information
	block_t* next;
	block_t* branch;

	// displaying information
	bool   visited;
	double x;
	double y;
	double h;
	double w;
};

size_t block_line(char* str, size_t size, instr_t* instr, size_t n_instr);

typedef struct
{
	block_t* b;
	size_t        n;
	size_t        a;
} blist_t;

void blist_new (blist_t* l);
void blist_del (blist_t* l);
void blist_push(blist_t* l, instr_t* start, size_t size);

block_t* blist_search(blist_t* l, size_t offset);

typedef struct
{
	block_t** f;
	size_t         n;
	size_t         a;
} functions_t;

void funs_new (functions_t* f);
void funs_del (functions_t* f);
void funs_push(functions_t* f, block_t* b);

#endif
