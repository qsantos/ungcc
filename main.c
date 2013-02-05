#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct instr
{
	unsigned long offset; // offset of the instruction
	const char*   orig;   // original instruction in dump
	unsigned long next;   // next instruction   (may be zero)
	unsigned long branch; // branch instruction (may be zero)
};

static unsigned long countOccurences(const char* str, char c)
{
	unsigned long ret = 0;
	for (; *str; str++)
		if (*str == c)
			ret++;
	return ret;
}

static void usage(int argc, char** argv)
{
	(void) argc;

	fprintf(stderr,
		"Usage: %s dumpfile\n"
		,
		argv[0]
	);
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		usage(argc, argv);
		exit(1);
	}

	int curarg = 1;
	FILE* input = fopen(argv[curarg++], "r");
	if (!input)
	{
		fprintf(stderr, "Could not open file '%s'\n", argv[curarg-1]);
		usage(argc, argv);
		exit(1);
	}

	size_t n_instr = 0;
	size_t a_instr = 0;
	struct instr* code = NULL;

	char*  line   = NULL;
	size_t n_line = 0;
	while (1)
	{
		getline(&line, &n_line, input);
		if (feof(input))
			break;

		// instruction
		if (line[0] != ' ')
			continue;

		char* part;
		part = strtok(line, "\t"); // address
		if (!part) continue;
		unsigned long offset = strtoul(part, NULL, 16) - 0x8048000;

		part = strtok(NULL, "\t"); // hexadecimal
		if (!part) continue;
		unsigned long length = countOccurences(part, ' ');

		part = strtok(NULL, "\t"); // assembly code
		if (!part) continue;

		if (n_instr == a_instr)
		{
			a_instr = a_instr ? 2*a_instr : 1;
			code = (struct instr*) realloc(code, sizeof(struct instr)*a_instr);
		}

		struct instr* cur = &code[n_instr++];
		cur->offset = offset;
		cur->orig   = strdup(part);

		part = strtok(part, " "); // opcode;
		if (strcmp(part, "jmp") == 0)
		{
			cur->next = 0;

			part = strtok(NULL, " "); // address
			cur->branch = strtoul(part, NULL, 16);
		}
		else
		{
			cur->next = offset + length;
			cur->branch = 0;
		}
	}
	fclose(input);

	for (size_t i = 0; i < n_instr; i++)
		printf("%s", code[i].orig);
	return 0;
}
