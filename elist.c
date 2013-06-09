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

char* read_register(reg_t* dst, size_t* sz, char* str)
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
		if      (str[0] == 'i' && str[1] == 'z') *dst = R_IZ;
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
		reg_t reg;
		str = read_register(&reg, sz, str+1);
		*dst = e_op_reg(reg);

		return str;
	}
	else if (str[0] == '$') // immediate
	{
		im_t im = strtoul(str+1, (char**) &str, 16);
		*dst = e_op_im(im);
		return str;
	}
	else if ('0' <= str[0] && str[0] <= '9' && str[1] != 'x') // immediate address
	{
		im_t im = strtoul(str, (char**) &str, 16);
		*dst = e_op_im(im);

		// read symbol
		if (strchr(str, '+') == NULL) // no offset
		{
			str += 2; // " <"
			char* end = strchr(str, '@');
			if (!end) end = strchr(str, '>');
			(*dst)->v.op.symbol = strndup(str, end - str);
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
		reg_t base = 0;
		str = read_register(&base, NULL, str+1);
		*dst = e_op_addr(base, 0, 0, 0);
		return str;
	}

	ssize_t disp = strtoul(str, (char**) &str, 16);

	if (str[0] != '(')
	{
		*dst = e_op_addr(0, 0, 0, disp);
		return str;
	}
	str++;

	reg_t base = 0;
	if (str[0] == '%')
		str = read_register(&base, NULL, str+1);

	if (str[0] != ',')
	{
		*dst = e_op_addr(base, 0, 0, disp);
		return str+1; // ')'
	}
	str++;

	reg_t idx;
	str = read_register(&idx, NULL, str+1);
	str++; // ','
	size_t scale = strtoul(str, (char**) &str, 10);

	*dst = e_op_addr(base, idx, scale, disp);
	return str+1; // ')'
}

#define ZER(N,T) if(strcmp(opcode,N)==0||strcmp(opcode,N"l")==0||strcmp(opcode,N"b")==0)\
	{elist_push(dst,of,T());   return;}
#define UNI(N,T) if(strcmp(opcode,N)==0||strcmp(opcode,N"l")==0||strcmp(opcode,N"b")==0)\
	{elist_push(dst,of,T(a));  return;}
#define BIN(N,T) if(strcmp(opcode,N)==0||strcmp(opcode,N"l")==0||strcmp(opcode,N"b")==0)\
	{elist_push(dst,of,T(b,a));return;}

#define UNI_F(N,T) if(strcmp(opcode,N)==0||strcmp(opcode,N"l")==0||strcmp(opcode,N"b")==0)\
	{elist_push(dst,of,e_mov(a,T(a)));   return;}
#define BIN_F(N,T) if(strcmp(opcode,N)==0||strcmp(opcode,N"l")==0||strcmp(opcode,N"b")==0)\
	{elist_push(dst,of,e_mov(b,T(b,a))); return;}

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
		elist_push(dst, of, e_mov (e_op_reg(R_BP), e_op_reg(R_SP)));
		elist_push(dst, of, e_push(e_op_reg(R_BP)));
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
	UNI("call", e_call)

	// unary affectation
	UNI_F("not" , e_not ) UNI("neg", e_neg)

	if (!str) return;
	expr_t* b = NULL;
	read_operand(&b, 0, str);

	// binary
	BIN("test", e_test) BIN("cmp", e_cmp)
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
	// finds _start()
	eopair_t* p = elist_at(l, entryPoint);
	if (!p)
		fprintf(stderr, "No such instruction: %#x\n\n", entryPoint);
	p->e->isFun = true;

	// finds main() address, right before the call to __libc_start_main@plt
	for (; p->e->type != E_CALL; p++);
	p--;
	expr_t* op = p->e->v.uni.a;
	if (p->e->type != E_PUSH || !op || op->type != E_OPERAND || op->v.op.t != IM)
	{
		fprintf(stderr, "Unexpected instruction:\n");
		fprint_stat(stderr, p->e);
		exit(1);
	}
	size_t mainAddr = p->e->v.uni.a->v.op.v.im;

	// builds hierarchy
	for (size_t i = 0; i < l->n; i++)
	{
		expr_t* e = l->e[i].e;

		if (e->type == E_JMP || e->type == E_RET || e->type == E_HLT)
			e->endBlck = true;
		else if (i+1 < l->n)
			e->next = l->e[i+1].e;

		// branch/function detection

		if (!(E_JMP <= e->type && e->type <= E_CALL)) // jump or call
			continue;

		expr_t* op = e->v.uni.a;
		if (op->type != E_OPERAND || op->v.op.t != IM)
		{
			fprintf(stderr, "Unsupported instruction at %#x: ", l->e[i].o);
			fprint_stat(stderr, e);
			continue;
		}

		eopair_t* p = elist_at(l, op->v.op.v.im);
		if (!p)
		{
			fprintf(stderr, "Unknown offset %#x\n", op->v.op.v.im);
			continue;
		}
		if (e->type == E_CALL)
			p->e->isFun = true;
		else
		{
			e->endBlck = true;
			e->branch = p->e;
			(p-1)->e->endBlck = true;
		}
	}

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
