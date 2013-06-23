/*\
 *  Binary analyzer for decompilation and forensics
 *  Copyright (C) 2013  Quentin SANTOS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/

#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <unistd.h>

#include "parser.h"
#include "postproc.h"
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

		// opens ELF file
		int input = open(filename, 0);
		if (input < 0)
		{
			fprintf(stderr, "Could not open file '%s'\n\n", filename);
			usage(name);
		}

		// reads ELF information
		elf_t* elf = elf_new();
		elf_begin(elf, input);

		elist_t el; // expression list
		flist_t fl; // function list

		read_file(&el, elf, f);
		post_funs(&fl, &el, elf);
		for (size_t i = 0; i < fl.n; i++)
		{
			expr_t* e = fl.f[i].expr;
			if (e == NULL)
				continue;
			post_rmctx(e);
			post_simpl(e);
			post_reduc(e);
		}
		zui(argc, argv, &fl);

		flist_del(&fl);
		elist_del(&el);

		elf_del(elf);
		close(input);

		fclose(f);
		return 0;
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
}
