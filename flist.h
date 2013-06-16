#ifndef FUNCTION_H
#define FUNCTION_H

typedef struct function function_t;

#include "expr.h"

struct function
{
	size_t  address;
	char*   name;
	expr_t* expr;
	bool    returns;
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
