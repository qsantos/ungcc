#include "toasm.h"

#include <string.h>

size_t regcode(const char* reg, const char** end)
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

const char* read_op(op* op, const char* str, size_t* s)
{
	if (str[0] == '%') // register
	{
		if (s && !*s) *s = regsize(str+1);
		asm_set_reg(op, regcode(str+1, &str));
		return str;
	}
	else if (str[0] == '$') // immediate
	{
		im im = strtoul(str+1, (char**) &str, 16);
		asm_set_im(op, im);
		return str;
	}
	else if ('0' <= str[0] && str[0] <= '9' && str[1] != 'x') // immediate address
	{
		im im = strtoul(str, (char**) &str, 16);
		asm_set_im(op, im);
		if (strchr(str, '+') == NULL) // no offset
		{
			str += 2; // " <"
			char* end = strchr(str, '>');
			op->symbol = strndup(str, end - str);
		}
		return str;
	}
	else if (str[0] == '*') // indirect address
	{
		str++;
	}

	// address
	ssize_t disp = strtoul(str, (char**) &str, 16);

	if (str[0] != '(')
	{
		asm_set_addr(op, 0, 0, 0, disp);
		return str;
	}
	str++;

	size_t base = regcode(str+1, &str);

	if (str[0] != ',')
	{
		asm_set_addr(op, base, 0, 0, disp);
		return str+1; // ')'
	}
	str++;

	size_t idx = regcode(str+1, &str);
	str++; // ','
	size_t scale = strtoul(str, (char**) &str, 10);

	asm_set_addr(op, base, idx, scale, disp);
	return str+1; // ')'
}
