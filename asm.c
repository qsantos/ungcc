#include "asm.h"

#include <stdio.h>
#include <string.h>

// BEGIN constructor, destructor

void asm_new(asm_t* asm)
{
	asm->i = NULL;
	asm->a = 0;
	asm->n = 0;
}

void asm_del(asm_t* asm)
{
	for (size_t i = 0; i < asm->n; i++)
	{
		free(asm->i[i].label);
		free(asm->i[i].orig);
	}
	free(asm->i);
	free(asm);
}

// END constructor, destructor

// BEGIN instruction building

instr_t* asm_next(asm_t* asm, size_t offset, char* orig, char* label)
{
	if (asm->n == asm->a)
	{
		asm->a = asm->a ? 2*asm->a : 1;
		asm->i = (instr_t*) realloc(asm->i, asm->a * sizeof(instr_t));
	}

	instr_t* i = &asm->i[asm->n++];
	i->offset   = offset;
	i->orig     = orig;
	i->label    = label;
	i->function = false;
	i->branch   = false;
	return i;
}

static int cmp_offset(const void* a, const void* b)
{
	instr_t* ia = (instr_t*) a;
	instr_t* ib = (instr_t*) b;

	if (ia->offset < ib->offset) return -1;
	if (ia->offset > ib->offset) return  1;
	return 0;
}
instr_t* asm_find_at(asm_t* asm, size_t offset)
{
	instr_t key;
	key.offset = offset;
	return bsearch(&key, asm->i, asm->n, sizeof(instr_t), cmp_offset);
}

void asm_set_reg(operand_t* op, reg_t reg)
{
	op->t = REG;
	op->v.reg = reg;
	op->symbol = NULL;
}

void asm_set_im(operand_t* op, im_t im)
{
	op->t = IM;
	op->v.im = im;
	op->symbol = NULL;
}

void asm_set_addr(operand_t* op, reg_t base, reg_t idx, im_t scale, im_t disp)
{
	op->t = ADDR;
	op->v.addr.base  = base;
	op->v.addr.idx   = idx;
	op->v.addr.scale = scale;
	op->v.addr.disp  = disp;
	op->symbol = NULL;
}

// END instruction building

// BEGIN parsing information from string

const char* read_register(reg_t* dst, size_t* sz, const char* str)
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
		if      (str[0] == 'i' && str[1] == 'z') *dst = 0 ;     // iz pseudo-strister (= 0)
		else if (str[1] == 'x') *dst = str[0] - 'a' + 1;        // ax, bx, cx, dx
		else if (str[1] == 'h') *dst = str[0] - 'a' + 1;        // ah, bh, ch, dh
		else if (str[1] == 'l') *dst = str[0] - 'a' + 5;        // al, bl, cl, dl
		else if (str[1] == 'p') *dst = str[0] == 's' ? 9 : 10;  // sp, bp
		else if (str[1] == 'i') *dst = str[0] == 's' ? 11 : 12; // si, di
		else                    *dst = 0;
	}

	return str+2;
}

const char* read_operand(operand_t* dst, size_t* sz, const char* str)
{
	if (str[0] == '%') // register
	{
		reg_t reg;
		str = read_register(&reg, sz, str+1);
		asm_set_reg(dst, reg);
		return str;
	}
	else if (str[0] == '$') // immediate
	{
		im_t im = strtoul(str+1, (char**) &str, 16);
		asm_set_im(dst, im);
		return str;
	}
	else if ('0' <= str[0] && str[0] <= '9' && str[1] != 'x') // immediate address
	{
		im_t im = strtoul(str, (char**) &str, 16);
		asm_set_im(dst, im);
		if (strchr(str, '+') == NULL) // no offset
		{
			str += 2; // " <"
			char* end = strchr(str, '>');
			dst->symbol = strndup(str, end - str);
		}
		return str;
	}
	else if (str[0] == '*') // indirect address
	{
		str++;
	}

	// address
	ssize_t disp = strtoul(str, (char**) &str, 16);

	if (str[0] != '(')
	{
		asm_set_addr(dst, 0, 0, 0, disp);
		return str;
	}
	str++;

	reg_t base = 0;
	if (str[0] == '%')
		str = read_register(&base, NULL, str+1);

	if (str[0] != ',')
	{
		asm_set_addr(dst, base, 0, 0, disp);
		return str+1; // ')'
	}
	str++;

	reg_t idx;
	str = read_register(&idx, NULL, str+1);
	str++; // ','
	size_t scale = strtoul(str, (char**) &str, 10);

	asm_set_addr(dst, base, idx, scale, disp);
	return str+1; // ')'
}

#define X(N) (sizeof(N)-1)
// instruction without parameters
#define READ_INSTR0(O,N) if (strcmp(opcode,N) == 0 || strcmp(opcode,N "l") == 0 || strcmp(opcode,N "b") == 0) \
	{ \
	i->op = O; \
	i->s = opcode[X(N)] == 'l' ? 32 : (opcode[X(N)] == 'b' ? 8 : 0); \
	continue; \
	}


// unary instruction
#define READ_INSTR1(O,N) if (strcmp(opcode,N) == 0 || strcmp(opcode,N "l") == 0 || strcmp(opcode,N "b") == 0) \
	{ \
	i->op = O; \
	i->s = opcode[X(N)] == 'l' ? 32 : (opcode[X(N)] == 'b' ? 8 : 0); \
	read_operand(&i->a, &i->s, params); \
	continue; \
	}

// binary instruction
#define READ_INSTR2(O,N) if (strcmp(opcode,N) == 0 || strcmp(opcode,N "l") == 0 || strcmp(opcode,N "b") == 0) \
	{ \
	i->op = O; \
	i->s = opcode[X(N)] == 'l' ? 32 : (opcode[X(N)] == 'b' ? 8 : 0); \
	params = read_operand(&i->a, &i->s, params) + 1; \
	read_operand(&i->a, &i->s, params); \
	continue; \
	}

void read_file(asm_t* asm, FILE* f)
{
	asm_new(asm);

	char* label = NULL;;
	char* line  = NULL;
	size_t n_line = 0;
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

		char* orig = strdup(part);
		instr_t* i = asm_next(asm, offset, orig, label);
		label = NULL;

		const char* opcode = part;
		while (*part && *part != ' ') part++; // skip the opcode
		if (*part)
		{
			*part = 0; part++;                    // mark the end of the opcode
			while (*part && *part == ' ') part++; // find the parameters
		}
		if (*part == 0) part = NULL; // no parameters

		const char* params = part;

		READ_INSTR0(NOP,   "nop");
		READ_INSTR0(RET,   "ret");
		READ_INSTR0(LEAVE, "leave");
		READ_INSTR0(HLT,   "hlt");
		READ_INSTR1(PUSH,  "push");
		READ_INSTR1(POP,   "pop");
		READ_INSTR1(JMP,   "jmp");
		READ_INSTR1(JE,    "je");
		READ_INSTR1(JNE,   "jne");
		READ_INSTR1(JA,    "ja");
		READ_INSTR1(JAE,   "jae");
		READ_INSTR1(JB,    "jb");
		READ_INSTR1(JBE,   "jbe");
		READ_INSTR1(JS,    "js");
		READ_INSTR1(JNS,   "jns");
		READ_INSTR1(JL,    "jl");
		READ_INSTR1(JLE,   "jle");
		READ_INSTR1(JG,    "jg");
		READ_INSTR1(JGE,   "jge");
		READ_INSTR1(CALL,  "call");
		READ_INSTR1(NOT,   "not");
		READ_INSTR1(NEG,   "neg");
		READ_INSTR2(ADD,   "add");
		READ_INSTR2(SUB,   "sub");
		READ_INSTR2(MUL,   "mul");
		READ_INSTR2(DIV,   "div");
		READ_INSTR2(AND,   "and");
		READ_INSTR2(OR,    "or");
		READ_INSTR2(XOR,   "xor");
		READ_INSTR2(SAR,   "sar");
		READ_INSTR2(SAL,   "sal");
		READ_INSTR2(SHR,   "shr");
		READ_INSTR2(SHL,   "shl");
		READ_INSTR2(TEST,  "test");
		READ_INSTR2(CMP,   "cmp");
		READ_INSTR2(XCHG,  "xchg");
		READ_INSTR2(MOV,   "mov");
		READ_INSTR2(LEA,   "lea");

		fprintf(stderr, "Unknown instruction '%s'\n", orig);
	}

	free(line);
}

// END parsing information from string

// BEGIN printing

#define PRTCHK(FCT, ...) {ret+=FCT(str+ret,size-ret,__VA_ARGS__);if(ret>=size)return ret;}

int print_reg(char* str, size_t size, reg_t reg, size_t s)
{
	size_t ret = 0;

	PRTCHK(snprintf, "%%");
	if (s == 8)
	{
		PRTCHK(snprintf, "%c%c", 'a' + ((reg-1)%4), reg < 4 ? 'h' : 'l');
		return ret;
	}

	if (s == 64)
		PRTCHK(snprintf, "s")
	else if (s == 32)
		PRTCHK(snprintf, "e")

	if (reg <= 4)
		PRTCHK(snprintf, "%cx", 'a'-1 + reg)
	else if (reg == 9)
		PRTCHK(snprintf, "sp")
	else if (reg == 10)
		PRTCHK(snprintf, "bp")
	else if (reg == 11)
		PRTCHK(snprintf, "si")
	else if (reg == 12)
		PRTCHK(snprintf, "di")
	else if (reg == 13)
		PRTCHK(snprintf, "zi")
	else
		PRTCHK(snprintf, "?")

	return ret;
}

int print_hex(char* str, size_t size, im_t im)
{
	size_t ret = 0;

	if (im < 0)
		PRTCHK(snprintf, "-%#x", -im)
	else
		PRTCHK(snprintf, "%#x", im)

	return ret;
}

int print_op(char* str, size_t size, operand_t* op, size_t s)
{
	size_t ret = 0;

	if (op->symbol)
	{
		PRTCHK(snprintf, "%s", op->symbol);
		return ret;
	}

	if (op->t == REG)
		PRTCHK(print_reg, op->v.reg, s)
	else if (op->t == IM)
	{
		size_t v = op->v.im;
		if (0x1f < v && v < 0x7f)
			PRTCHK(snprintf, "'%c'", v)
		else if (v < 0x8048000)
			PRTCHK(snprintf, "%zi", v)
		else
		{
			PRTCHK(snprintf, "$");
			PRTCHK(print_hex, v);
		}
	}
	else if (op->t == ADDR)
	{
		addr_t* a = &op->v.addr;
		if (a->disp)
			PRTCHK(print_hex, a->disp);
		if (a->base)
		{
			PRTCHK(snprintf, "(");
			PRTCHK(print_reg, a->base, 32);
			if (a->scale)
			{
				PRTCHK(snprintf, ",");
				PRTCHK(print_reg, a->idx, 32);
				PRTCHK(snprintf, ", %u", a->scale);
			}
			PRTCHK(snprintf, ")");
		}
	}

	return ret;
}

#define PRINT_INSTR0(O,N) if(i->op==O){PRTCHK(snprintf, N);}

#define PRINT_INSTR1(O,N) if(i->op==O){PRTCHK(snprintf, N " ");\
	PRTCHK(print_op, &i->a,i->s);}

#define PRINT_INSTR2(O,N) if(i->op==O){PRTCHK(snprintf, N " ");\
	PRTCHK(print_op, &i->a,i->s);\
	PRTCHK(snprintf, ",");\
	PRTCHK(print_op, &i->b,i->s);}

int print_instr(char* str, size_t size, instr_t* i)
{
	size_t ret = 0;
	*str = 0;

	if (i->label)
		PRTCHK(snprintf, "<%s>:\n", i->label)

	if (i->op == UNK)
		PRTCHK(snprintf, "=> %s", i->orig)

	PRINT_INSTR0(NOP,   "nop");
	PRINT_INSTR0(RET,   "ret");
	PRINT_INSTR0(LEAVE, "leave");
	PRINT_INSTR0(HLT,   "hlt");
	PRINT_INSTR1(PUSH,  "push");
	PRINT_INSTR1(POP,   "pop");
	PRINT_INSTR1(JMP,   "jmp");
	PRINT_INSTR1(JE,    "je");
	PRINT_INSTR1(JNE,   "jne");
	PRINT_INSTR1(JA,    "ja");
	PRINT_INSTR1(JAE,   "jae");
	PRINT_INSTR1(JB,    "jb");
	PRINT_INSTR1(JBE,   "jbe");
	PRINT_INSTR1(JS,    "js");
	PRINT_INSTR1(JNS,   "jns");
	PRINT_INSTR1(JL,    "jl");
	PRINT_INSTR1(JLE,   "jle");
	PRINT_INSTR1(JG,    "jg");
	PRINT_INSTR1(JGE,   "jge");
	PRINT_INSTR1(CALL,  "call");
	PRINT_INSTR1(NOT,   "not");
	PRINT_INSTR1(NEG,   "neg");
	PRINT_INSTR2(ADD,   "add");
	PRINT_INSTR2(SUB,   "sub");
	PRINT_INSTR2(MUL,   "mul");
	PRINT_INSTR2(DIV,   "div");
	PRINT_INSTR2(AND,   "and");
	PRINT_INSTR2(OR,    "or");
	PRINT_INSTR2(XOR,   "xor");
	PRINT_INSTR2(SAR,   "sar");
	PRINT_INSTR2(SAL,   "sal");
	PRINT_INSTR2(SHR,   "shr");
	PRINT_INSTR2(SHL,   "shl");
	PRINT_INSTR2(TEST,  "test");
	PRINT_INSTR2(CMP,   "cmp");
	PRINT_INSTR2(XCHG,  "xchg");
	PRINT_INSTR2(MOV,   "mov");
	PRINT_INSTR2(LEA,   "lea");

	PRTCHK(snprintf, "\n");

	return ret;
}

void fprint_instr(FILE* f, instr_t* i)
{
	char buffer[1024];
	print_instr(buffer, 1024, i);
	fprintf(f, "%s", buffer);
}

// END printing
