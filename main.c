#include "toasm.h"
#include "block.h"

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

	// reads dump
	struct asm asm;
	fromfile(&asm, input);
	fclose(input);

	// reads _start() to find main() address
	struct instr* i = offset2instr(&asm, entryPoint);
	if (!i)
		fprintf(stderr, "No such instruction: %#x\n\n", entryPoint);
	i->function = true;
	for (; i->op != CALL; i++);
	i--;
	if (i->op != PUSH || i->a.t != IM)
	{
		fprintf(stderr, "Unexpected instruction:\n");
		instr_print(i);
		exit(1);
	}
	size_t mainAddr = i->a.v.im;
	i = offset2instr(&asm, mainAddr);
	if (!i)
	{
		fprintf(stderr, "Could not find main() at address %#x\n", mainAddr);
		exit(1);
	}
	i->function = true;

	// mark called functions
	for (size_t k = 0; k < asm.n; k++)
	{
		struct instr* i = asm.i + k;
		if (i->a.t != IM)
			continue;

		if (i->op == CALL)
		{
			if ((i = offset2instr(&asm, i->a.v.im)))
				i->function = true;
		}
		else if (i->op == JMP ||
		         i->op == JE  || i->op == JNE ||
		         i->op == JA  || i->op == JB  ||
			 i->op == JS  || i->op == JNS ||
		         i->op == JL  || i->op == JLE)
		{
			if ((i = offset2instr(&asm, i->a.v.im)))
				i->branch = true;
		}
	}

	// find blocks
	struct blist blist;
	blist_new(&blist);

	size_t start = 0;
	for (size_t k = 1; k < asm.n; k++)
	{
		struct instr* i = asm.i + k;

		size_t end;
		if (i->function || i->branch)
			end = k-1;
		else if (i->op == RET || i->op == LEAVE ||
		         i->op == HLT || i->op == JMP   ||
		         i->op == JE  || i->op == JNE   ||
		         i->op == JA  || i->op == JB    ||
			 i->op == JS  || i->op == JNS   ||
		         i->op == JL  || i->op == JLE)
		{
			end = k;
		}
		else
			continue;

		if (start >= end)
			continue;

		blist_push(&blist, asm.i + start, end-start);

		printf("Block:\n");
		for (size_t k = start; k <= end; k++)
			instr_print(asm.i + k);
		printf("\n\n");
		start = end+1;
	}

	blist_del(&blist);

	return 0;
}
