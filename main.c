#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "code.h"

size_t regcode(char* reg, char** end)
{
	if (reg[0] == 'e' || reg[0] == 'r')
		reg++;
	
	if (end) *end = reg+2;

	if (reg[1] == 'x') // ax, bx, cx, dx
		return reg[0] - 'a';

	if (reg[1] == 'h') // ah, bh, ch, dh
		return reg[0] - 'a';

	if (reg[1] == 'l') // al, bl, cl, dl
		return reg[0] - 'a' + 4;
	
	if (reg[1] == 'p') // sp, bp
		return reg[0] == 's' ? 8 : 9;

	if (reg[1] == 'i') // si, di
		return reg[0] == 's' ? 10 : 11;
	
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

char* read_op(op* op, char* str, size_t* s)
{
	if (str[0] == '%') // register
	{
		if (s && !*s) *s = regsize(str+1);
		code_set_reg(op, regcode(str+1, &str));
		return str;
	}
	else if (str[0] == '$') // immediate
	{
		im im = strtoul(str+1, &str, 16);
		code_set_im(op, im);
		return str;
	}
	else if ('0' <= str[0] && str[0] <= '9' && str[1] != 'x') // immediate address
	{
		im im = strtoul(str, &str, 16);
		code_set_im(op, im);
		return str;
	}
	else if (str[0] == '*') // indirect address
	{
		str++;
	}

	// address
	ssize_t disp = strtoul(str, &str, 16);

	if (str[0] != '(')
	{
		code_set_addr(op, 0, 0, 0, disp);
		return str;
	}
	str++;

	size_t base = regcode(str+1, &str);

	if (str[0] != ',')
	{
		code_set_addr(op, base, 0, 0, disp);
		return str+1; // ')'
	}
	str++;

	size_t idx = regcode(str+1, &str);
	str++; // ','
	size_t scale = strtoul(str, &str, 10);

	code_set_addr(op, base, idx, scale, disp);
	return str+1; // ')'
}

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

	struct code code;
	code_new(&code);

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
		struct instr* i = code_next(&code, offset, orig);

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

	code_print(&code);

	return 0;
}
