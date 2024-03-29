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

#include "parser.h"

#include <stdlib.h>
#include <string.h>

char* read_register(expr_reg_type_t* dst, int* sz, elf_t* elf, char* str)
{
	(void) elf;

	// register size
	if (sz)
	{
		if      (str[0] == 'r') *sz = 64;
		else if (str[0] == 'e') *sz = 32;
		else if (str[1] == 'h' ||
		         str[1] == 'l') *sz = 8;
		else                    *sz = 16;
	}

	if (str[0] == 'e' || str[0] == 'r')
		str++;

	// register code
	if (dst)
	{
		if (str[0] == 's' && str[1] == 't') // FPU register
		{
			if (str[2] == '(')
			{
				*dst = R_ST0 + (str[3] - '0');
				return str+5;
			}
			else
				*dst = R_ST0;
		}
		else if (str[0] == 'i' && str[1] == 'z') *dst = R_IZ;
		else if (str[1] == 'x') *dst = R_AX + (str[0] - 'a');  // ax, bx, cx, dx
		else if (str[1] == 'l') *dst = R_AL + (str[0] - 'a');  // al, bl, cl, dl
		else if (str[1] == 'h') *dst = R_AH + (str[0] - 'a');  // ah, bh, ch, dh
		else if (str[1] == 'p') *dst = str[0] == 's' ? R_SP : R_BP;
		else if (str[1] == 'i') *dst = str[0] == 's' ? R_SI : R_DI;
		else
		{
			fprintf(stderr, "Unknown register '%s'\n", str);
			*dst = 0;
		}
	}

	return str+2;
}

char* read_operand(expr_t** dst, int* sz, elf_t* elf, char* str)
{
	if (str[0] == '%') // register
	{
		expr_reg_type_t reg;
		str = read_register(&reg, sz, elf, str+1);
		*dst = e_reg(reg);
		return str;
	}
	else if (str[0] == '$') // immediate
	{
		value_t im = strtoul(str+1, (char**) &str, 16);
		*dst = e_im(im);
		(*dst)->v.im.str = elf_str(elf, im); // TODO
		return str;
	}
	else if ('0' <= str[0] && str[0] <= '9' && str[1] != 'x') // immediate address
	{
		value_t im = strtoul(str, (char**) &str, 16);
		*dst = e_im(im);
		return str;
	}
	else if (str[0] == '*') // indirect address
	{
		str++;
	}

	// indirect address
	if (str[0] == '%')
	{
		expr_reg_type_t base = 0;
		str = read_register(&base, NULL, elf, str+1);
		*dst = e_addr(base, 0, 0, 0);
		return str;
	}

	value_t disp = strtoul(str, (char**) &str, 16);

	if (str[0] != '(')
	{
		*dst = e_addr(0, 0, 0, disp);
		return str;
	}
	str++;

	expr_reg_type_t base = 0;
	if (str[0] == '%')
		str = read_register(&base, NULL, elf, str+1);

	if (str[0] != ',')
	{
		*dst = e_addr(base, 0, 0, disp);
		return str+1; // ')'
	}
	str++;

	expr_reg_type_t idx;
	str = read_register(&idx, NULL, elf, str+1);
	str++; // ','
	value_t scale = strtoul(str, (char**) &str, 10);

	*dst = e_addr(base, idx, scale, disp);
	return str+1; // ')'
}

#define ZER(N,T) if(strcmp(opcode,N)==0||strcmp(opcode,N"l")==0||strcmp(opcode,N"b")==0)\
	{elist_push(dst,of,T());   return;}
#define UNI(N,T) if(strcmp(opcode,N)==0||strcmp(opcode,N"l")==0||strcmp(opcode,N"b")==0)\
	{elist_push(dst,of,T(a));  return;}
#define BIN(N,T) if(strcmp(opcode,N)==0||strcmp(opcode,N"l")==0||strcmp(opcode,N"b")==0)\
	{elist_push(dst,of,T(b,a));return;}

#define JXX(N,T) if(strcmp(opcode,N)==0)\
	{elist_push(dst,of,e_jxx(a, e_test(T, e_reg(R_FL))));return;}

#define UNI_F(N,T) if(strcmp(opcode,N)==0||strcmp(opcode,N"l")==0||strcmp(opcode,N"b")==0)\
	{elist_push(dst,of,e_mov(e_cpy(a),T(a)));   return;}
#define BIN_F(N,T) if(strcmp(opcode,N)==0||strcmp(opcode,N"l")==0||strcmp(opcode,N"b")==0)\
	{elist_push(dst,of,e_mov(e_cpy(b),T(b,a))); return;}

void read_instr(elist_t* dst, address_t of, elf_t* elf, char* str)
{
	const char* opcode = str;
	while (*str && *str != ' ') str++; // skip the opcode
	if (*str)
	{
		*str = 0; str++;                   // mark the end of the opcode
		while (*str && *str == ' ') str++; // find the parameters
	}
	if (*str == 0) str = NULL; // no parameters

	// zeroary
	ZER("nop", e_nop)
	ZER("ret", e_ret)
	ZER("hlt", e_hlt)

	if (strcmp(opcode, "leave") == 0)
	{
		elist_push(dst, of, e_mov (e_reg(R_BP), e_reg(R_SP)));
		elist_push(dst, of, e_push(e_reg(R_BP))); // TODO: two instructions with same offset
		return;
	}

	if (!str) return;
	expr_t* a = NULL;
	str = read_operand(&a, NULL, elf, str)+1;

	// unary
	UNI("push", e_push) UNI("pop", e_pop)
	UNI("jmp" , e_jmp )

	JXX("je", T_E) JXX("jne", T_NE)
	JXX("js", T_S) JXX("jns", T_NS)
	JXX("ja", T_A) JXX("jae", T_AE)
	JXX("jb", T_B) JXX("jbe", T_BE)

	if (strcmp(opcode, "call") == 0)
	{
		elist_push(dst, of, e_mov(e_reg(R_AX), e_call(a)));
		return;
	}

	// unary affectation
	UNI_F("not" , e_not ) UNI("neg", e_neg)

	if (!str) return;
	expr_t* b = NULL;
	read_operand(&b, 0, elf, str);

	// binary
	if (strcmp(opcode, "test") == 0)
	{
		elist_push(dst, of, e_mov(e_reg(R_FL), e_and(b, a)));
		return;
	}
	if (strcmp(opcode, "cmp") == 0 || strcmp(opcode, "cmpl") == 0 || strcmp(opcode, "cmpb") == 0)
	{
		elist_push(dst, of, e_mov(e_reg(R_FL), e_sub(b, a)));
		return;
	}
	BIN("xchg", e_xchg) BIN("mov", e_mov) BIN("lea", e_lea)

	// binary affectation
	BIN_F("add" , e_add ) BIN_F("sub", e_sub) BIN_F("sbb", e_sbb) BIN_F("mul", e_mul) BIN_F("div", e_div)
	BIN_F("and" , e_and ) BIN_F("or" , e_or ) BIN_F("xor", e_xor)
	BIN_F("sar" , e_sar ) BIN_F("sal", e_sal) BIN_F("shr", e_shr) BIN_F("shl", e_shl)

	fprintf(stderr, "Unknown instruction '%s'\n", opcode);
	elist_push(dst, of, e_unk(strdup(opcode)));
}

void read_file(elist_t* dst, elf_t* elf, FILE* f)
{
	elist_new(dst);

	char*   line   = NULL;
	size_t  n_line = 0;
	while (1)
	{
		ssize_t n_read = getline(&line, &n_line, f);
		if (feof(f))
			break;
		line[n_read-1] = 0;

		// not instruction
		if (line[0] != ' ')
			continue;

		char* part;
		part = strtok(line, "\t"); // address
		if (!part) continue;
		address_t offset = strtoul(part, NULL, 16);

		part = strtok(NULL, "\t"); // hexadecimal value
		if (!part) continue;

		part = strtok(NULL, "\t"); // assembly code
		if (!part) continue;

		read_instr(dst, offset, elf, part);
	}

	free(line);
}
