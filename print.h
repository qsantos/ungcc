#ifndef PRINT_H
#define PRINT_H

#include <stdio.h>

#include "expr.h"

size_t print_reg (char* str, size_t size, rtype_t reg, size_t s);
size_t print_hex (char* str, size_t size, ssize_t v);
size_t print_expr(char* str, size_t size, expr_t* e);
size_t print_stat(char* str, size_t size, expr_t* i);
void  fprint_stat(FILE* f,   expr_t* e);

#endif
