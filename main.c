#include <time.h>

#include "block.h"
#include "interface.h"

static void usage(const char* name)
{
	fprintf(stderr,
		"Usage: %s entryPoint [dumpFile]\n"
		, name);
	exit(1);
}

int main(int argc, char** argv)
{
	srand(time(NULL));

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
	asm_t asm;
	read_file(&asm, input);
	fclose(input);

	// reads _start() to find main() address
	instr_t* i = asm_find_at(&asm, entryPoint);
	if (!i)
		fprintf(stderr, "No such instruction: %#x\n\n", entryPoint);
	i->function = true;
	for (; i->op != O_CALL; i++);
	i--;
	if (i->op != O_PUSH || i->a.t != IM)
	{
		fprintf(stderr, "Unexpected instruction:\n");
		fprint_instr(stderr, i);
		exit(1);
	}
	size_t mainAddr = i->a.v.im;
	i = asm_find_at(&asm, mainAddr);
	if (!i)
	{
		fprintf(stderr, "Could not find main() at address %#x\n", mainAddr);
		exit(1);
	}
	i->function = true;

	// mark block and function beginnings
	for (size_t k = 0; k < asm.n; k++)
	{
		instr_t* i = asm.i + k;
		if (i->a.t != IM)
			continue;

		if (i->op == O_CALL)
		{
			if ((i = asm_find_at(&asm, i->a.v.im)))
				i->function = true;
		}
		else if (O_JMP <= i->op && i->op <= O_JGE) // jump instruction
		{
			if ((i = asm_find_at(&asm, i->a.v.im)))
				i->branch = true;
		}
	}

	// find blocks
	blist_t blist;
	blist_new(&blist);
	size_t start = 0;
	for (size_t k = 1; k < asm.n; k++)
	{
		instr_t* i = asm.i + k;

		size_t end;
		if (i->function || i->branch) // block beginning
			end = k-1;
		else if ((O_RET <= i->op && i->op <= O_HLT) || // function end // TODO
		         (O_JMP <= i->op && i->op <= O_JGE))   // jump instruction
		{
			end = k;
		}
		else
			continue;

		if (start > end)
			continue;

		blist_push(&blist, asm.i + start, end-start+1);
		start = end+1;
	}

	// find hierarchy
	functions_t funs;
	funs_new(&funs);
	for (size_t k = 0; k < blist.n; k++)
	{
		block_t* b = blist.b + k;
		if (b->start->function)
			funs_push(&funs, b);
		instr_t* i = b->start + b->size-1;

		b->branch = NULL;
		if (O_JMP <= i->op && i->op <= O_JGE && i->a.t == IM) // jump instruction at immediate address
		{
			b->branch = blist_search(&blist, i->a.v.im);

			if (!b->branch)
			{
				fprintf(stderr, "Instruction jumps to unknown offset %#x\n", i->a.v.im);
				fprint_instr(stderr, i);
			}
		}

		if ((i->op <= O_RET && i->op <= O_HLT) || i->op == O_JMP) // function end or inconditionnal jump
			b->next = NULL;
		else if (k < blist.n-1 && (b+1)->start->function == false)
			b->next = b+1;
		else
			b->next = NULL;
	}

	size_t k = 1;//rand() % funs.n;
	block_t* fun = funs.f[k];
	size_t len = (k < funs.n ? funs.f[k+1] : blist.b+blist.n) - fun;
	zui(argc, argv, fun, len);

	blist_del(&blist);
	return 0;
}
