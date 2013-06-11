#include "elist.h"

#include <stdlib.h>
#include <string.h>

#include "print.h"

void elist_new(elist_t* l)
{
	l->e = NULL;
	l->n = 0;
	l->a = 0;
}

void elist_del(elist_t* l)
{
	// TODO
	free(l->e);
}

void elist_push(elist_t* l, size_t o, expr_t* e)
{
	if (l->n == l->a)
	{
		l->a = l->a ? 2*l->a : 1;
		l->e = (eopair_t*) realloc(l->e, l->a * sizeof(eopair_t));
	}
	l->e[l->n].o = o;
	l->e[l->n].e = e;
	l->n++;
}

static int cmp_eopair(const void* a, const void* b)
{
	const eopair_t* eoa = (const eopair_t*) a;
	const eopair_t* eob = (const eopair_t*) b;

	if (eoa->o < eob->o) return -1;
	if (eoa->o > eob->o) return  1;
	                     return  0;
}
eopair_t* elist_at(elist_t* l, size_t o)
{
	eopair_t key = {o, NULL};
	return bsearch(&key, l->e, l->n, sizeof(eopair_t), cmp_eopair);
}

char* read_register(rtype_t* dst, size_t* sz, char* str)
{
	// register size
	if (sz)
	{
		if      (str[0] == 'r') *sz = 64;
		else if (str[0] == 'e') *sz = 32;
		else if (str[1] == 'h' ||
		         str[1] == 'l') *sz = 8;
		else                    *sz = 16;
	}

	if (str[0] == 'e' || str[0] == 'r')
		str++;

	// register code
	if (dst)
	{
		if (str[0] == 's' && str[1] == 't') // FPU register
		{
			if (str[2] == '(')
			{
				*dst = R_ST0 + (str[3] - '0');
				return str+5;
			}
			else
				*dst = R_ST0;
		}
		else if (str[0] == 'i' && str[1] == 'z') *dst = R_IZ;
		else if (str[1] == 'x') *dst = R_AX + (str[0] - 'a');  // ax, bx, cx, dx
		else if (str[1] == 'l') *dst = R_AL + (str[0] - 'a');  // al, bl, cl, dl
		else if (str[1] == 'h') *dst = R_AH + (str[0] - 'a');  // ah, bh, ch, dh
		else if (str[1] == 'p') *dst = str[0] == 's' ? R_SP : R_BP;
		else if (str[1] == 'i') *dst = str[0] == 's' ? R_SI : R_DI;
		else
		{
			fprintf(stderr, "Unknown register '%s'\n", str);
			*dst = 0;
		}
	}

	return str+2;
}

char* read_operand(expr_t** dst, size_t* sz, char* str)
{
	if (str[0] == '%') // register
	{
		rtype_t reg;
		str = read_register(&reg, sz, str+1);
		*dst = e_reg(reg);

		return str;
	}
	else if (str[0] == '$') // immediate
	{
		size_t im = strtoul(str+1, (char**) &str, 16);
		*dst = e_im(im);
		return str;
	}
	else if ('0' <= str[0] && str[0] <= '9' && str[1] != 'x') // immediate address
	{
		size_t im = strtoul(str, (char**) &str, 16);
		*dst = e_im(im);

		// read symbol
		if (strchr(str, '+') == NULL) // no offset
		{
			str += 2; // " <"
			char* end = strchr(str, '@');
			if (!end) end = strchr(str, '>');
			(*dst)->v.im.symbol = strndup(str, end - str);
		}
		return str;
	}
	else if (str[0] == '*') // indirect address
	{
		str++;
	}

	// indirect address
	if (str[0] == '%')
	{
		rtype_t base = 0;
		str = read_register(&base, NULL, str+1);
		*dst = e_addr(base, 0, 0, 0);
		return str;
	}

	ssize_t disp = strtoul(str, (char**) &str, 16);

	if (str[0] != '(')
	{
		*dst = e_addr(0, 0, 0, disp);
		return str;
	}
	str++;

	rtype_t base = 0;
	if (str[0] == '%')
		str = read_register(&base, NULL, str+1);

	if (str[0] != ',')
	{
		*dst = e_addr(base, 0, 0, disp);
		return str+1; // ')'
	}
	str++;

	rtype_t idx;
	str = read_register(&idx, NULL, str+1);
	str++; // ','
	size_t scale = strtoul(str, (char**) &str, 10);

	*dst = e_addr(base, idx, scale, disp);
	return str+1; // ')'
}

#define ZER(N,T) if(strcmp(opcode,N)==0||strcmp(opcode,N"l")==0||strcmp(opcode,N"b")==0)\
	{elist_push(dst,of,T());   return;}
#define UNI(N,T) if(strcmp(opcode,N)==0||strcmp(opcode,N"l")==0||strcmp(opcode,N"b")==0)\
	{elist_push(dst,of,T(a));  return;}
#define BIN(N,T) if(strcmp(opcode,N)==0||strcmp(opcode,N"l")==0||strcmp(opcode,N"b")==0)\
	{elist_push(dst,of,T(b,a));return;}

#define UNI_F(N,T) if(strcmp(opcode,N)==0||strcmp(opcode,N"l")==0||strcmp(opcode,N"b")==0)\
	{elist_push(dst,of,e_mov(e_cpy(a),T(a)));   return;}
#define BIN_F(N,T) if(strcmp(opcode,N)==0||strcmp(opcode,N"l")==0||strcmp(opcode,N"b")==0)\
	{elist_push(dst,of,e_mov(e_cpy(b),T(b,a))); return;}

void read_instr(elist_t* dst, size_t of, char* str)
{
	const char* opcode = str;
	while (*str && *str != ' ') str++; // skip the opcode
	if (*str)
	{
		*str = 0; str++;                   // mark the end of the opcode
		while (*str && *str == ' ') str++; // find the parameters
	}
	if (*str == 0) str = NULL; // no parameters

	// zeroary
	ZER("nop", e_nop)
	ZER("ret", e_ret)
	ZER("hlt", e_hlt)

	if (strcmp(opcode, "leave") == 0)
	{
		elist_push(dst, of, e_mov (e_reg(R_BP), e_reg(R_SP)));
		elist_push(dst, of, e_push(e_reg(R_BP))); // TODO: two instructions with same offset
		return;
	}

	if (!str) return;
	expr_t* a = NULL;
	str = read_operand(&a, NULL, str)+1;

	// unary
	UNI("push", e_push) UNI("pop", e_pop)
	UNI("jmp" , e_jmp )
	UNI("je"  , e_je  ) UNI("jne", e_jne)
	UNI("js"  , e_js  ) UNI("jns", e_jns)
	UNI("ja"  , e_ja  ) UNI("jae", e_jae)
	UNI("jb"  , e_jb  ) UNI("jbe", e_jbe)
	UNI("jl"  , e_jl  ) UNI("jle", e_jle)
	UNI("jg"  , e_jg  ) UNI("jge", e_jge)

	if (strcmp(opcode, "call") == 0)
	{
		elist_push(dst, of, e_mov(e_reg(R_AX), e_call(a)));
		return;
	}

	// unary affectation
	UNI_F("not" , e_not ) UNI("neg", e_neg)

	if (!str) return;
	expr_t* b = NULL;
	read_operand(&b, 0, str);

	// binary
	if (strcmp(opcode, "test") == 0)
	{
		elist_push(dst, of, e_mov(e_reg(R_FL), e_and(b, a)));
		return;
	}
	if (strcmp(opcode, "cmp") == 0 || strcmp(opcode, "cmpl") == 0 || strcmp(opcode, "cmpb") == 0)
	{
		elist_push(dst, of, e_mov(e_reg(R_FL), e_sub(b, a)));
		return;
	}
	BIN("xchg", e_xchg) BIN("mov", e_mov) BIN("lea", e_lea)

	// binary affectation
	BIN_F("add" , e_add ) BIN_F("sub", e_sub) BIN_F("sbb", e_sbb) BIN_F("mul", e_mul) BIN_F("div", e_div)
	BIN_F("and" , e_and ) BIN_F("or" , e_or ) BIN_F("xor", e_xor)
	BIN_F("sar" , e_sar ) BIN_F("sal", e_sal) BIN_F("shr", e_shr) BIN_F("shl", e_shl)

	fprintf(stderr, "Unknown instruction '%s'\n", opcode);
}

void read_file(elist_t* dst, FILE* f)
{
	elist_new(dst);

	char*   label  = NULL;
	char*   line   = NULL;
	size_t  n_line = 0;
	while (1)
	{
		size_t n_read = getline(&line, &n_line, f);
		if (feof(f))
			break;
		line[n_read-1] = 0;

		// label
		if (line[0] == '0')
		{
			label = strchr(line, '<')+1;
			char* end = strchr(label , '>');
			*end = 0;
			label = strdup(label);
			continue;
		}

		// not instruction
		if (line[0] != ' ')
			continue;

		char* part;
		part = strtok(line, "\t"); // address
		if (!part) continue;
		size_t offset = strtoul(part, NULL, 16);

		part = strtok(NULL, "\t"); // hexadecimal value
		if (!part) continue;

		part = strtok(NULL, "\t"); // assembly code
		if (!part) continue;

		size_t cur = dst->n;
		read_instr(dst, offset, part);
		// adds label to first added instruction
		if (cur < dst->n)
		{
			dst->e[cur].e->label = label;
			label = NULL;
		}
	}

	free(line);
}

size_t functions(elist_t* dst, elist_t* l, size_t entryPoint)
{
	// builds hierarchy
	for (size_t i = 0; i < l->n; i++)
	{
		expr_t* e = l->e[i].e;

		if (e->type == E_JMP || e->type == E_RET || e->type == E_HLT)
			e->endBlck = true;
		else if (i+1 < l->n)
			e->next = l->e[i+1].e;

		if (!(E_JMP <= e->type && e->type <= E_JGE)) // any jump
			continue;

		expr_t* a = e->v.uni.a;
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

	// find functions
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

		eopair_t* p = elist_at(l, b->v.im.v);
		if (!p)
			continue;
		p->e->isFun = true;
	}

	// finds _start()
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

	// lists functions
	elist_new(dst);
	size_t ret = 0;
	for (size_t i = 0; i < l->n; i++)
	{
		expr_t* e = l->e[i].e;
		size_t  o = l->e[i].o;
		if (o == mainAddr)
			ret = dst->n;
		if (e->isFun)
		{
			elist_push(dst, o, e);
			if (i)
			{
				l->e[i-1].e->next = NULL;
				l->e[i-1].e->endBlck = true;
			}
		}
	}

	return ret;
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
void stripcontext(expr_t* e)
{
	for (; isContextInit(e); e = e->next)
	{
		e_del(e, true);
		e->type = E_NOP;
	}

	reset_visited(e);
	isContextEnd(e);
	reset_visited(e); // HACK: unexpected behaviour, does not reset 'visited' in postproc() without this
}

#define POST1(T) case T: postproc_aux1(e->v.uni.a); break;
#define POST2(T) case T: postproc_aux1(e->v.bin.a); postproc_aux1(e->v.bin.b); break;

static void postproc_aux1(expr_t* e)
{
	switch (e->type)
	{
	// unary
	POST1(E_PUSH); POST1(E_POP);
	POST1(E_JMP);
	POST1(E_JE);   POST1(E_JNE);
	POST1(E_JS);   POST1(E_JNS);
	POST1(E_JA);   POST1(E_JAE);
	POST1(E_JB);   POST1(E_JBE);
	POST1(E_JL);   POST1(E_JLE);
	POST1(E_JG);   POST1(E_JGE);
	POST1(E_CALL);
	POST1(E_NOT);  POST1(E_NEG);

	// binary
	POST2(E_ADD);  POST2(E_SUB); POST2(E_SBB); POST2(E_MUL); POST2(E_DIV);
	POST2(E_OR);
	POST2(E_SAR);  POST2(E_SAL); POST2(E_SHR); POST2(E_SHL);
	POST2(E_XCHG); POST2(E_MOV); POST2(E_LEA);

	case E_AND:
	{
		expr_t* a = e->v.bin.a; postproc_aux1(a);
		expr_t* b = e->v.bin.b; postproc_aux1(b);
		if (cmp_expr(a, b) == 0)
		{
			free(e->label);
			e_del(b, false);
			memcpy(e, a, sizeof(expr_t));
		}
		break;
	}
	case E_XOR:
	{
		expr_t* a = e->v.bin.a; postproc_aux1(a);
		expr_t* b = e->v.bin.b; postproc_aux1(b);
		if (cmp_expr(a, b) == 0)
		{
			e->type = E_IM;
			e->v.im.v = 0;
			e->v.im.symbol = NULL;
			e_del(a, false);
			e_del(b, false);
		}
		break;
	}

	default:
		break;
	}
}
static void postproc_aux2(expr_t* e)
{
	if (e == NULL || e->visited)
		return;
	e->visited = true;

	postproc_aux1(e);

	postproc_aux2(e->next);
	postproc_aux2(e->branch);
}
void postproc(expr_t* e)
{
	reset_visited(e);
	postproc_aux2(e);
}

#define REDUC1(T) case T: reduc_aux1(r, e->v.uni.a, last); break;
#define REDUC2(T) case T: reduc_aux1(r, e->v.bin.a, last); reduc_aux1(r, e->v.bin.b, last); break;

static void reduc_aux1(expr_t* r, expr_t* e, expr_t** last)
{
	switch (e->type)
	{
	case E_REG:
	{
		expr_t* l = last[e->v.reg.t];
		if (l == NULL)
			break;
		if (l->next == r) // to avoid reordering
			e->v.reg.last = l;
		l->used++;
		break;
	}

	// unary
	REDUC1(E_PUSH); REDUC1(E_POP);
	REDUC1(E_JMP);
	REDUC1(E_JE);   REDUC1(E_JNE);
	REDUC1(E_JS);   REDUC1(E_JNS);
	REDUC1(E_JA);   REDUC1(E_JAE);
	REDUC1(E_JB);   REDUC1(E_JBE);
	REDUC1(E_JL);   REDUC1(E_JLE);
	REDUC1(E_JG);   REDUC1(E_JGE);
	REDUC1(E_CALL);
	REDUC1(E_NOT);  REDUC1(E_NEG);

	// binary
	REDUC2(E_ADD);  REDUC2(E_SUB); REDUC2(E_SBB); REDUC2(E_MUL); REDUC2(E_DIV);
	REDUC2(E_AND);  REDUC2(E_OR);  REDUC2(E_XOR);
	REDUC2(E_SAR);  REDUC2(E_SAL); REDUC2(E_SHR); REDUC2(E_SHL);

	case E_MOV:
	case E_LEA:
		reduc_aux1(r, e->v.bin.b, last);
		break;

	default:
		break;
	}
}
static void reduc_aux2(expr_t* e, expr_t** last)
{
	if (e == NULL || e->visited)
		return;
	e->visited = true;

	reduc_aux1(e, e, last);

	expr_t* a = e->v.bin.a;
	if (e->type == E_MOV && a->type == E_REG)
	{
		rtype_t reg = a->v.reg.t;
		expr_t* prev = last[reg];
		last[reg] = e;
		reduc_aux2(e->next, last);
		reduc_aux2(e->branch, last);
		last[reg] = prev;
	}
	else
	{
		reduc_aux2(e->next, last);
		reduc_aux2(e->branch, last);
	}
}
void reduc(expr_t* e)
{
	expr_t* last[N_REG];
	memset(last, 0, N_REG * sizeof(expr_t*));
	reset_visited(e);
	reduc_aux2(e, last);
}
