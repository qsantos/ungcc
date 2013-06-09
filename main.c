#include <stdlib.h>
#include <time.h>

#include "elist.h"
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

	elist_t el; // expression list
	elist_t fl; // function list

	read_file(&el, input);
	size_t main_idx = functions(&fl, &el, entryPoint);
	expr_t* e = fl.e[main_idx].e;
	postproc(e);
	reduc(e);
	zui(argc, argv, e);

	elist_del(&fl);
	elist_del(&el);

	return 0;
}
