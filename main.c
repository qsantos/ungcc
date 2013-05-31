#include "toasm.h"

/*
static size_t countOccurences(const char* str, char c)
{
	size_t ret = 0;
	for (; *str; str++)
		if (*str == c)
			ret++;
	return ret;
}
*/

static void usage(const char* name)
{
	fprintf(stderr,
		"Usage: %s entryPoint [dumpFile]\n"
		, name);
	exit(1);
}

int main(int argc, char** argv)
{
	const char* name = argv[0];

	if (argc == 1)
		usage(name);

	// argument handling
	int argn = 1;

	if (argn == argc)
	{
		fprintf(stderr, "Missing entry point\n\n");
		usage(name);
	}
	const char* entry_str = argv[argn++];
	size_t entryPoint = strtoul(entry_str, NULL, 0);

	FILE* input = argc >= 3 ? fopen(argv[argn], "r") : stdin;
	if (!input)
	{
		fprintf(stderr, "Could not open file '%s'\n\n", argv[argn]);
		usage(name);
	}
	argn++;

	// reads dump
	struct asm asm;
	fromfile(&asm, input);
	fclose(input);

	// mark functions
	struct instr* i = offset2instr(&asm, entryPoint);
	if (!i)
	{
		fprintf(stderr, "No such instruction: %#x\n\n", entryPoint);
		usage(name);
	}
	i->function = true;
	for (size_t k = 0; k < asm.n; k++)
	{
		struct instr* i = asm.i + k;
		if (i->op == CALL && i->a.t == IM)
		{
			i = offset2instr(&asm, i->a.v.im);
			if (i)
				i->function = true;
		}
	}

	// print functions
	for (size_t k = 0; k < asm.n; k++)
	{
		struct instr* i = asm.i + k;
		if (i->function)
			instr_print(i);
	}

//	static const opcode blockEnd[] = {RET, LEAVE, HLT, JMP, JE, JNE, JA, JB, JS, JL, JLE};

//	asm_print(&asm);

/*
	bool incrIP = false;
	for (size_t k = 0; k < asm.n; k++)
	{
		struct instr* i = asm.i + k;

		if (incrIP)
			i->function = false;

		if (i->op == JMP || i->op == RET || i->op == LEAVE || i->op == HLT)
			incrIP = false;
		else if (i->op != NOP)
			incrIP = true;

		if (JMP <= i->op && i->op <= JLE && i->a.t == IM)
		{
			i = offset2instr(&asm, i->a.v.im);
			if (i)
				i->function = false;
		}
	}

	for (size_t k = 0; k < asm.n; k++)
		if (asm.i[k].function)
			instr_print(asm.i+k);
*/

	return 0;
}
