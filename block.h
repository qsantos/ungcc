#ifndef BLOCK_H
#define BLOCK_H

#include "asm.h"

struct block
{
	// block information
	struct instr* start;
	size_t        size;

	// branching information
	struct block* next;
	struct block* branch;

	// displaying information
	bool  drawn;
	double x;
	double y;
};

struct blist
{
	struct block* b;
	size_t        n;
	size_t        a;
};

void blist_new (struct blist* l);
void blist_del (struct blist* l);
void blist_push(struct blist* l, struct instr* start, size_t size);

struct block* blist_search(struct blist* l, size_t offset);

struct functions
{
	struct block** f;
	size_t         n;
	size_t         a;
};

void funs_new (struct functions* f);
void funs_del (struct functions* f);
void funs_push(struct functions* f, struct block* b);

#endif
