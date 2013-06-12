#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <unistd.h>

#include "elist.h"
#include "interface.h"

static void usage(const char* name)
{
	fprintf(stderr,
		"Usage: %s ELFFile\n"
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
		fprintf(stderr, "Missing file name\n\n");
		usage(name);
	}
	const char* filename = argv[argn++];



	// open executable
	int input = open(filename, 0);
	if (input < 0)
	{
		fprintf(stderr, "Could not open file '%s'\n\n", filename);
		usage(name);
	}

	// read ELF information
	elf_t* elf = elf_new();
	elf_begin(elf, input);
	size_t entryPoint = elf_entry(elf);

	// uses objdump to get .text assembly
	int fifo[2];
	pipe(fifo);
	if (fork()) // reading dump
	{
		close(fifo[1]);

		FILE* f = fdopen(fifo[0], "r");
		if (f == NULL)
		{
			fprintf(stderr, "Error when converting fifo file descriptor to FILE\n");
			exit(1);
		}

		elist_t el; // expression list
		elist_t fl; // function list

		read_file(&el, elf, f);
		functions(&fl, &el, entryPoint);
		for (size_t i = 0; i < fl.n; i++)
		{
			expr_t* e = fl.e[i].e;
			stripcontext(e);
			postproc(e);
			//reduc(e);
		}
		zui(argc, argv, &fl);

		elist_del(&fl);
		elist_del(&el);
	}
	else // outputting dump
	{
		dup2(fifo[1], STDOUT_FILENO);
		close(fifo[0]);

		const char* objdump = "/usr/bin/objdump";
		execl(objdump, objdump, "-j", ".text", "-d", filename, NULL);
		fprintf(stderr, "Callind '%s' failed\n", objdump);
		exit(1);
	}

	elf_del(elf);
	close(input);
	return 0;
}
