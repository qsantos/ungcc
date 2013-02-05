#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
		part = strtok(NULL, "\t"); // hexadecimal
		if (!part) continue;
		part = strtok(NULL, "\t"); // assembly code
		if (!part) continue;

		size_t len = strlen(part);
		part[len-1] = 0;
		printf("%s\n", part);
	}

	fclose(input);
	return 0;
}
