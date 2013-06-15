#include "postproc.h"

#include <stdlib.h>
#include <string.h>

#include "print.h"

void post_funs(flist_t* dst, elist_t* l, elf_t* elf)
{
	// builds hierarchy
	for (size_t i = 0; i < l->n; i++)
	{
		expr_t* e = l->e[i].e;

		if (e->type == E_JMP || e->type == E_RET || e->type == E_HLT)
			e->endBlck = true;
		else if (i+1 < l->n)
			e->next = l->e[i+1].e;

		if (e->type != E_JMP && e->type != E_JXX)
			continue;

		expr_t* a = e->v.bin.a;
		if (a->type != E_IM)
		{
			fprintf(stderr, "Unsupported instruction at %#x: ", l->e[i].o);
			fprint_stat(stderr, e);
			continue;
		}

		eopair_t* p = elist_at(l, a->v.im.v);
		if (!p)
		{
			fprintf(stderr, "Unknown offset %#x\n", a->v.im.v);
			continue;
		}

		e->endBlck = true;
		e->branch = p->e;
		(p-1)->e->endBlck = true;
	}

	// finds _start()
	size_t entryPoint = elf_entry(elf);
	eopair_t* p = elist_at(l, entryPoint);
	if (!p)
		fprintf(stderr, "No such instruction: %#x\n\n", entryPoint);
	p->e->isFun = true;

	// finds main() address, right before the call to __libc_start_main@plt
	for (; !(p->e->type == E_MOV && p->e->v.bin.b->type == E_CALL); p++);
	p--;
	expr_t* a = p->e->v.uni.a;
	if (p->e->type != E_PUSH || !a || a->type != E_IM)
	{
		fprintf(stderr, "Unexpected instruction:\n");
		fprint_stat(stderr, p->e);
		exit(1);
	}
	size_t mainAddr = a->v.im.v;

	// marks main() as a function
	p = elist_at(l, mainAddr);
	if (!p)
	{
		fprintf(stderr, "Could not find main() at address %#x\n", mainAddr);
		exit(1);
	}
	p->e->isFun = true;

	// finds called functions
	flist_t tmp_fl;
	flist_new(&tmp_fl);
	for (size_t i = 0; i < l->n; i++)
	{
		expr_t* e = l->e[i].e;

		// ax = fct()
		if (e->type != E_MOV)
			continue;

		expr_t* a = e->v.bin.b;
		if (a->type != E_CALL)
			continue;

		expr_t* b = a->v.uni.a;
		if (b->type != E_IM)
		{
			fprintf(stderr, "Unsupported instruction at %#x: ", l->e[i].o);
			fprint_stat(stderr, a);
			continue;
		}

		size_t address = b->v.im.v;
		eopair_t* p = elist_at(l, address);
		if (p)
		{
			// the function is defined in .text, it will be
			// saved later, with corresponding expression
			p->e->isFun = true;
		}
		else
		{
			// this is an external function
			flist_push(&tmp_fl, address, NULL);
		}
	}

	// lists local functions
	for (size_t i = 0; i < l->n; i++)
	{
		expr_t* e = l->e[i].e;
		size_t  o = l->e[i].o;
		if (e->isFun)
		{
			flist_push(&tmp_fl, o, e);
			if (e->label == NULL)
			{
				if (o == mainAddr)
					e->label = "main";
			}
			if (i)
			{
				l->e[i-1].e->next = NULL;
				l->e[i-1].e->endBlck = true;
			}
		}
	}

	// now extract sorted and named uniq function symbols
	flist_sort(&tmp_fl);
	flist_new(dst);
	size_t last_addr = 0;
	size_t unnamed_idx = 0;
	for (size_t i = 0; i < tmp_fl.n; i++)
	{
		function_t* fun = tmp_fl.f + i;
		size_t addr = fun->address;

		if (addr == last_addr)
			continue;
		last_addr = addr;

		fun = flist_push(dst, addr, fun->expr);
		if (fun->name)
			continue;

		fun->name = elf_plt(elf, addr);
		if (fun->name)
			continue;

		fun->name = elf_sym(elf, addr);
		if (fun->name)
			continue;

		char buf[1024];
		snprintf(buf, 1024, "fct%u", ++unnamed_idx);
		fun->name = strdup(buf);
	}
	flist_del(&tmp_fl);

	// assigns function symbols to calls and jumps
	for (size_t i = 0; i < l->n; i++)
	{
		expr_t* e = l->e[i].e;

		expr_t* b;
		if (e->type == E_JMP)
			b = e->v.bin.a;
		else if (e->type == E_MOV)
		{
			expr_t* a = e->v.bin.b;
			if (a->type != E_CALL)
				continue;
			b = a->v.uni.a;
		}
		else
			continue;

		if (b->type != E_IM)
			continue;

		b->v.im.sym = flist_find(dst, b->v.im.v);
	}
}


static bool isContextInit(expr_t* e)
{
	if (e->type == E_PUSH)
	{
		return true;
	}
	if (e->type == E_MOV)
	{
		expr_t* a = e->v.bin.a;
		expr_t* b = e->v.bin.b;
		if (a->type == E_REG && b->type == E_REG)
		{
			rtype_t reg_a = a->v.reg.t;
			rtype_t reg_b = b->v.reg.t;
			return reg_a == R_BP && reg_b == R_SP;
		}
		else
			return a->type == E_REG && isContextInit(b);

	}
	if (e->type == E_AND)
	{
		expr_t* a = e->v.bin.a;
		expr_t* b = e->v.bin.b;
		return a->type == E_REG && a->v.reg.t == R_SP && b->type == E_IM;
	}
	if (e->type == E_SUB)
	{
		expr_t* a = e->v.bin.a;
		expr_t* b = e->v.bin.b;
		return a->type == E_REG && a->v.reg.t == R_SP && b->type == E_IM;
	}
	return false;
}
static bool isContextEnd(expr_t* e)
{
	if (e == NULL)  return true;
	if (e->visited) return false;
	e->visited = true;

	bool nextIsContext = isContextEnd(e->next);
	isContextEnd(e->branch);

	if (e->branch == NULL && nextIsContext)
	{
		if (e->type == E_RET)
			return true;
		if (e->type == E_POP)
		{
			e_del(e, true);
			e->type = E_NOP;
			return true;
		}
		return false;
	}

	return false;
}
void post_rmctx(expr_t* e)
{
	for (; isContextInit(e); e = e->next)
	{
		e_del(e, true);
		e->type = E_NOP;
	}

	e_rstvisited(e);
	isContextEnd(e);
	e_rstvisited(e); // HACK: unexpected behaviour, does not reset 'visited' in postproc() without this
}

static void post_simpl_aux1(expr_t* e)
{
	if (e == NULL) return;

	switch (e->type)
	{
	// unary
	case E_PUSH: case E_POP:
	case E_CALL:
	case E_NOT:  case E_NEG:
		post_simpl_aux1(e->v.uni.a);
		break;

	// binary
	case E_JMP:  case E_JXX:
	case E_ADD:  case E_SUB: case E_SBB: case E_MUL: case E_DIV:
	case E_OR:
	case E_SAR:  case E_SAL: case E_SHR: case E_SHL:
	case E_XCHG: case E_MOV:
		post_simpl_aux1(e->v.bin.a);
		post_simpl_aux1(e->v.bin.b);
		break;

	case E_AND:
	{
		expr_t* a = e->v.bin.a; post_simpl_aux1(a);
		expr_t* b = e->v.bin.b; post_simpl_aux1(b);
		if (e_cmp(a, b) == 0)
		{
			free(e->label);
			e_del(b, false);
			memcpy(e, a, sizeof(expr_t));
		}
		break;
	}
	case E_XOR:
	{
		expr_t* a = e->v.bin.a; post_simpl_aux1(a);
		expr_t* b = e->v.bin.b; post_simpl_aux1(b);
		if (e_cmp(a, b) == 0)
		{
			e->type = E_IM;
			e->v.im.v   = 0;
			e->v.im.sym = NULL;
			e->v.im.str = NULL;
			e_del(a, false);
			e_del(b, false);
		}
		break;
	}
	case E_LEA:
	{
		expr_t* a = e->v.bin.a; post_simpl_aux1(a);
		expr_t* b = e->v.bin.b; post_simpl_aux1(b);
		if (b->type == E_ADDR && b->v.addr.disp == 0)
		{
			rtype_t base  = b->v.addr.base;
			rtype_t idx   = b->v.addr.idx;
			size_t  scale = b->v.addr.scale;

			e->type = E_MOV;
			if (idx == base)
			{
				e->v.bin.b = e_mul(e_im(scale+1), e_reg(idx));
			}
			else if (idx != R_IZ)
			{
				expr_t* nb = e_reg(idx);
				if (scale != 1)    nb = e_mul(e_im(scale), nb);
				if (base  != R_IZ) nb = e_add(e_reg(base), nb);
				e->v.bin.b = nb;
			}
			else if (base != R_IZ)
			{
				if (a->type == E_REG && a->v.reg.t == base)
				{
					e->type = E_NOP;
					e_del(a, false);
				}
				else
					e->v.bin.b = e_reg(base);
			}
			else
				e->v.bin.b = e_im(0);

			e_del(b, false);
		}
		break;
	}

	case E_TEST:
		post_simpl_aux1(e->v.test.a);
		break;

	default:
		break;
	}
}
static void post_simpl_aux2(expr_t* e)
{
	if (e == NULL || e->visited)
		return;
	e->visited = true;

	post_simpl_aux1(e);

	post_simpl_aux2(e->next);
	post_simpl_aux2(e->branch);
}
void post_simpl(expr_t* e)
{
	e_rstvisited(e);
	post_simpl_aux2(e);
}

static void post_reduc_aux1(expr_t* r, expr_t* e, expr_t** last)
{
	if (e == NULL) return;

	switch (e->type)
	{
	case E_REG:
	{
		expr_t* l = last[e->v.reg.t];
		if (l == NULL)
			break;
		if (l->next == r) // to avoid reordering
			e->v.reg.last = l;
		else
			l->used++; // in doubt, block it
		l->used++;
		break;
	}

	// unary
	case E_PUSH: case E_POP:
	case E_CALL:
	case E_NOT:  case E_NEG:
		post_reduc_aux1(r, e->v.uni.a, last);
		break;

	// binary
	case E_JMP: case E_JXX:
	case E_ADD: case E_SUB: case E_SBB: case E_MUL: case E_DIV:
	case E_AND: case E_OR:  case E_XOR:
	case E_SAR: case E_SAL: case E_SHR: case E_SHL:
		post_reduc_aux1(r, e->v.bin.a, last);
		post_reduc_aux1(r, e->v.bin.a, last);
		break;

	case E_MOV:
	case E_LEA:
		post_reduc_aux1(r, e->v.bin.b, last);
		break;

	case E_TEST:
		post_reduc_aux1(r, e->v.test.a, last);
		break;

	default:
		break;
	}
}
static void post_reduc_aux2(expr_t* e, expr_t** last)
{
	if (e == NULL || e->visited)
		return;
	e->visited = true;

	post_reduc_aux1(e, e, last);

	expr_t* a = e->v.bin.a;
	if (e->type == E_MOV && a->type == E_REG)
	{
		rtype_t reg = a->v.reg.t;
		expr_t* prev = last[reg];
		last[reg] = e;
		post_reduc_aux2(e->next, last);
		post_reduc_aux2(e->branch, last);
		last[reg] = prev;
	}
	else
	{
		post_reduc_aux2(e->next, last);
		post_reduc_aux2(e->branch, last);
	}
}
void post_reduc(expr_t* e)
{
	expr_t* last[N_REG];
	memset(last, 0, N_REG * sizeof(expr_t*));
	e_rstvisited(e);
	post_reduc_aux2(e, last);
}
