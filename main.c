#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "toasm.h"

/*
static unsigned long countOccurences(const char* str, char c)
{
	unsigned long ret = 0;
	for (; *str; str++)
		if (*str == c)
			ret++;
	return ret;
}
*/

static void usage(const char* name)
{
	fprintf(stderr,
		"Usage: %s dumpfile\n"
		, name);
	exit(1);
}

int main(int argc, char** argv)
{
	if (argc < 2)
		usage(argv[0]);

	int curarg = 1;
	FILE* input = fopen(argv[curarg++], "r");
	if (!input)
	{
		fprintf(stderr, "Could not open file '%s'\n", argv[curarg-1]);
		usage(argv[0]);
	}

	struct asm asm;
	asm_new(&asm);

	char*  line   = NULL;
	size_t n_line = 0;
	while (1)
	{
		size_t n_read = getline(&line, &n_line, input);
		if (feof(input))
			break;
		line[n_read-1] = 0;

		// instruction
		if (line[0] != ' ')
			continue;

		char* part;
		part = strtok(line, "\t"); // address
		if (!part) continue;
		unsigned long offset = strtoul(part, NULL, 16) - 0x8048000;

		part = strtok(NULL, "\t"); // hexadecimal value
		if (!part) continue;
//		unsigned long length = countOccurences(part, ' ');

		part = strtok(NULL, "\t"); // assembly code
		if (!part) continue;

		char* orig = strdup(part);
		struct instr* i = asm_next(&asm, offset, orig);

		const char* opcode = part;
		while (*(++part) != ' '); // skip the opcode
		*part = 0;                // mark the end of the opcode
		while (*(++part) == ' '); // find the parameters
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
		READ_INSTR1(JB,    "jb");
		READ_INSTR1(JS,    "js");
		READ_INSTR1(JL,    "jl");
		READ_INSTR1(JLE,   "jle");
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
		READ_INSTR2(LEA,   "lea");

		if (strncmp(opcode, "mov", 3) == 0) // mov, movb, movl
		{
			i->op = MOV;

			if (opcode[3] == 'l')
				i->s = 32;
			else if (opcode[3] == 'b')
				i->s = 8;
			else
				i->s = 0;

			params = read_op(&i->a, params, &i->s)+1;
			read_op(&i->b, params, &i->s);
		}
//		cur->next = offset + length;
	}
	fclose(input);

	asm_print(&asm);

	return 0;
}
