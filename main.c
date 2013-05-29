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
	fromfile(&asm, input);
	fclose(input);

	asm_print(&asm);

	return 0;
}
