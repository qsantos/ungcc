#include "toasm.h"

#include <string.h>

// TODO: should be an enum
size_t regcode(const char* reg, const char** end)
{
	if (reg[0] == 'e' || reg[0] == 'r')
		reg++;
	
	if (end) *end = reg+2;

	if (reg[1] == 'x') // ax, bx, cx, dx
		return reg[0] - 'a' + 1;

	if (reg[1] == 'h') // ah, bh, ch, dh
		return reg[0] - 'a' + 1;

	if (reg[1] == 'l') // al, bl, cl, dl
		return reg[0] - 'a' + 5;
	
	if (reg[1] == 'p') // sp, bp
		return reg[0] == 's' ? 9 : 10;

	if (reg[1] == 'i') // si, di
		return reg[0] == 's' ? 11 : 12;
	
	if (reg[0] == 'i' && reg[1] == 'z') // eiz pseudo-register (= 0)
		return 13;

	return 0;
}

size_t regsize(const char* reg)
{
	if (reg[0] == 'r')
		return 64;
	if (reg[0] == 'e')
		return 32;
	if (reg[1] == 'h' || reg[1] == 'l')
		return 8;
	return 16;
}

const char* read_op(op* op, const char* str, size_t* s)
{
	if (str[0] == '%') // register
	{
		if (s && !*s) *s = regsize(str+1);
		asm_set_reg(op, regcode(str+1, &str));
		return str;
	}
	else if (str[0] == '$') // immediate
	{
		im im = strtoul(str+1, (char**) &str, 16);
		asm_set_im(op, im);
		return str;
	}
	else if ('0' <= str[0] && str[0] <= '9' && str[1] != 'x') // immediate address
	{
		im im = strtoul(str, (char**) &str, 16);
		asm_set_im(op, im);
		if (strchr(str, '+') == NULL) // no offset
		{
			str += 2; // " <"
			char* end = strchr(str, '>');
			op->symbol = strndup(str, end - str);
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
		asm_set_addr(op, 0, 0, 0, disp);
		return str;
	}
	str++;

	
	size_t base = str[0] == '%' ? regcode(str+1, &str) : 0;

	if (str[0] != ',')
	{
		asm_set_addr(op, base, 0, 0, disp);
		return str+1; // ')'
	}
	str++;

	size_t idx = regcode(str+1, &str);
	str++; // ','
	size_t scale = strtoul(str, (char**) &str, 10);

	asm_set_addr(op, base, idx, scale, disp);
	return str+1; // ')'
}

#define X(N) (sizeof(N)-1)
// instruction without parameters
#define READ_INSTR0(O,N) if (strcmp(opcode,N) == 0 || strcmp(opcode,N "l") == 0|| strcmp(opcode,N "b") == 0) \
	{ \
	i->op = O; \
	i->s = opcode[X(N)] == 'l' ? 32 : (opcode[X(N)] == 'b' ? 8 : 0); \
	continue; \
	}


// unary instruction
#define READ_INSTR1(O,N) if (strcmp(opcode,N) == 0 || strcmp(opcode,N "l") == 0|| strcmp(opcode,N "b") == 0) \
	{ \
	i->op = O; \
	i->s = opcode[X(N)] == 'l' ? 32 : (opcode[X(N)] == 'b' ? 8 : 0); \
	read_op(&i->a,params,&i->s); \
	continue; \
	} 

// binary instruction
#define READ_INSTR2(O,N) if (strcmp(opcode,N) == 0 || strcmp(opcode,N "l") == 0|| strcmp(opcode,N "b") == 0) \
	{ \
	i->op = O; \
	i->s = opcode[X(N)] == 'l' ? 32 : (opcode[X(N)] == 'b' ? 8 : 0); \
	params = read_op(&i->a,params,&i->s) + 1; \
	read_op(&i->b,params,&i->s); \
	continue; \
	}

void fromfile(struct asm* asm, FILE* f)
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
		struct instr* i = asm_next(asm, offset, orig, label);
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
		READ_INSTR2(XCHG,  "xchg");
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
		READ_INSTR2(MOV,   "mov");
		READ_INSTR2(LEA,   "lea");

		fprintf(stderr, "Unknown instruction '%s'\n", orig);
	}

	free(line);
}
