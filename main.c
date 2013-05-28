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

		part = strtok(part, " "); // opcode;
		if (strncmp(part, "mov", 3) == 0) // mov, movb, movl
		{
			i->op = MOV;

			if (part[3] == 'l')
				i->s = 32;
			else if (part[3] == 'b')
				i->s = 8;
			else
				i->s = 0;

			// source
			part = strtok(NULL, " ");
			part = read_op(&i->a, part, &i->s);
			part++; // ','

			// destination
			read_op(&i->b, part, &i->s);
		}
		else if (strcmp(part, "lea") == 0)
		{
			i->op = LEA;

			// source
			part = strtok(NULL, " ");
			part = read_op(&i->a, part, &i->s);
			part++; // ','

			// destination
			read_op(&i->b, part, &i->s);
		}
		else if (strcmp(part, "call") == 0)
		{
			i->op = CALL;

			part = strtok(NULL, " ");
			read_op(&i->a, part, NULL);
		}
		else if (strcmp(part, "jmp") == 0)
		{
			i->op = JMP;

			part = strtok(NULL, " ");
			read_op(&i->a, part, NULL);
		}
		else if (strcmp(part, "je") == 0)
		{
			i->op = JE;

			part = strtok(NULL, " ");
			read_op(&i->a, part, NULL);
		}
		else if (strcmp(part, "jne") == 0)
		{
			i->op = JNE;

			part = strtok(NULL, " ");
			read_op(&i->a, part, NULL);
		}
/*
*/
//			cur->next = offset + length;
	}
	fclose(input);

	asm_print(&asm);

	return 0;
}
