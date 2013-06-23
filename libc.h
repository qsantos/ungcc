/*\
 *  This is an awesome programm simulating awesome battles of awesome robot tanks
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

static const function_t fctInfo[] =
{
	// function name,  returns, argc,  fast,    0, NULL
	{ "abort",           false,    0, false,    0, NULL },
	{ "abs",              true,    1, false,    0, NULL },
	{ "asctime",          true,    1, false,    0, NULL },
	{ "atexit",           true,    1, false,    0, NULL },
	{ "atof",             true,    1, false,    0, NULL },
	{ "atoi",             true,    1, false,    0, NULL },
	{ "atol",             true,    1, false,    0, NULL },
	{ "bsearch",          true,    5, false,    0, NULL },
	{ "calloc",           true,    2, false,    0, NULL },
	{ "clearerr",         true,    1, false,    0, NULL },
	{ "clock",            true,    0, false,    0, NULL },
	{ "ctime",            true,    1, false,    0, NULL },
	{ "difftime",         true,    0, false,    0, NULL }, // TODO
	{ "div",              true,    2, false,    0, NULL },
	{ "exit",            false,    1, false,    0, NULL },
	{ "fclose",           true,    1, false,    0, NULL },
	{ "feof",             true,    1, false,    0, NULL },
	{ "ferror",           true,    1, false,    0, NULL },
	{ "fflush",           true,    1, false,    0, NULL },
	{ "fgetc",            true,    1, false,    0, NULL },
	{ "fgetpos",          true,    2, false,    0, NULL },
	{ "fgets",            true,    3, false,    0, NULL },
	{ "fgetwc",           true,    1, false,    0, NULL },
	{ "fgetws",           true,    3, false,    0, NULL },
	{ "fopen",            true,    2, false,    0, NULL },
	{ "fprintf",          true,    0, false,    0, NULL }, // TODO
	{ "fputc",            true,    2, false,    0, NULL },
	{ "fputs",            true,    2, false,    0, NULL },
	{ "fputwc",           true,    2, false,    0, NULL },
	{ "fputws",           true,    2, false,    0, NULL },
	{ "fread",            true,    4, false,    0, NULL },
	{ "free",             true,    1, false,    0, NULL },
	{ "freopen",          true,    3, false,    0, NULL },
	{ "frexp",            true,    2, false,    0, NULL },
	{ "fscanf",           true,    0, false,    0, NULL }, // TODO
	{ "fseek",            true,    3, false,    0, NULL },
	{ "fsetpos",          true,    2, false,    0, NULL },
	{ "ftell",            true,    1, false,    0, NULL },
	{ "fwprintf",         true,    0, false,    0, NULL }, // TODO
	{ "fwrite",           true,    4, false,    0, NULL },
	{ "fwscanf",          true,    0, false,    0, NULL }, // TODO
	{ "getc",             true,    1, false,    0, NULL },
	{ "getchar",          true,    0, false,    0, NULL },
	{ "getenv",           true,    1, false,    0, NULL },
	{ "gets",             true,    1, false,    0, NULL },
	{ "getwchar",         true,    0, false,    0, NULL },
	{ "getwc",            true,    1, false,    0, NULL },
	{ "gmtime",           true,    0, false,    0, NULL },
	{ "isalnum",          true,    1, false,    0, NULL },
	{ "isalpha",          true,    1, false,    0, NULL },
	{ "iscntrl",          true,    1, false,    0, NULL },
	{ "isdigit",          true,    1, false,    0, NULL },
	{ "isgraph",          true,    1, false,    0, NULL },
	{ "islower",          true,    1, false,    0, NULL },
	{ "isprint",          true,    1, false,    0, NULL },
	{ "ispunct",          true,    1, false,    0, NULL },
	{ "isspace",          true,    1, false,    0, NULL },
	{ "isupper",          true,    1, false,    0, NULL },
	{ "iswalnum",         true,    1, false,    0, NULL },
	{ "iswalpha",         true,    1, false,    0, NULL },
	{ "iswcntrl",         true,    1, false,    0, NULL },
	{ "iswctype",         true,    1, false,    0, NULL },
	{ "iswdigit",         true,    1, false,    0, NULL },
	{ "iswgraph",         true,    1, false,    0, NULL },
	{ "iswlower",         true,    1, false,    0, NULL },
	{ "iswprint",         true,    1, false,    0, NULL },
	{ "iswpunct",         true,    1, false,    0, NULL },
	{ "iswspace",         true,    1, false,    0, NULL },
	{ "iswupper",         true,    1, false,    0, NULL },
	{ "iswxdigit",        true,    1, false,    0, NULL },
	{ "isxdigit",         true,    1, false,    0, NULL },
	{ "labs",             true,    1, false,    0, NULL },
	{ "ldexp",            true,    2, false,    0, NULL },
	{ "ldiv",             true,    2, false,    0, NULL },
	{ "localeconv",       true,    0, false,    0, NULL },
	{ "localtime",        true,    1, false,    0, NULL },
	{ "longjmp",          true,    0, false,    0, NULL }, // TODO
	{ "malloc",           true,    1, false,    0, NULL },
	{ "mblen",            true,    2, false,    0, NULL },
	{ "mbrlen",           true,    3, false,    0, NULL },
	{ "mbrtowc",          true,    4, false,    0, NULL },
	{ "mbsinit",          true,    1, false,    0, NULL },
	{ "mbsrtowcs",        true,    4, false,    0, NULL },
	{ "mbstowcs",         true,    3, false,    0, NULL },
	{ "mbtowc",           true,    3, false,    0, NULL },
	{ "memchr",           true,    2, false,    0, NULL },
	{ "memcmp",           true,    3, false,    0, NULL },
	{ "memcpy",           true,    3, false,    0, NULL },
	{ "memmove",          true,    3, false,    0, NULL },
	{ "memset",           true,    3, false,    0, NULL },
	{ "mktime",           true,    1, false,    0, NULL },
	{ "modf",             true,    2, false,    0, NULL },
	{ "perror",           true,    1, false,    0, NULL },
	{ "printf",           true,    0, false,    0, NULL }, // TODO
	{ "putc",             true,    2, false,    0, NULL },
	{ "putchar",          true,    1, false,    0, NULL },
	{ "puts",             true,    1, false,    0, NULL },
	{ "putwchar",         true,    1, false,    0, NULL },
	{ "putwc",            true,    2, false,    0, NULL },
	{ "qsort",            true,    4, false,    0, NULL },
	{ "raise",            true,    1, false,    0, NULL },
	{ "rand",             true,    0, false,    0, NULL },
	{ "realloc",          true,    2, false,    0, NULL },
	{ "remove",           true,    1, false,    0, NULL },
	{ "rename",           true,    2, false,    0, NULL },
	{ "rewind",           true,    1, false,    0, NULL },
	{ "scanf",            true,    0, false,    0, NULL }, // TODO
	{ "setbuf",           true,    2, false,    0, NULL },
	{ "setjmp",           true,    0, false,    0, NULL }, // TODO
	{ "setlocale",        true,    2, false,    0, NULL },
	{ "setvbuf",          true,    4, false,    0, NULL },
	{ "signal",           true,    0, false,    0, NULL }, // TODO
	{ "sprintf",          true,    0, false,    0, NULL }, // TODO
	{ "srand",            true,    1, false,    0, NULL },
	{ "sscanf",           true,    0, false,    0, NULL }, // TODO
	{ "strcat",           true,    2, false,    0, NULL },
	{ "strchr",           true,    2, false,    0, NULL },
	{ "strcmp",           true,    2, false,    0, NULL },
	{ "strcoll",          true,    2, false,    0, NULL },
	{ "strcpy",           true,    2, false,    0, NULL },
	{ "strcspn",          true,    2, false,    0, NULL },
	{ "strerror",         true,    1, false,    0, NULL },
	{ "strftime",         true,    4, false,    0, NULL },
	{ "strlen",           true,    1, false,    0, NULL },
	{ "strncat",          true,    3, false,    0, NULL },
	{ "strncmp",          true,    3, false,    0, NULL },
	{ "strncpy",          true,    3, false,    0, NULL },
	{ "strpbrk",          true,    2, false,    0, NULL },
	{ "strrchr",          true,    2, false,    0, NULL },
	{ "strspn",           true,    2, false,    0, NULL },
	{ "strstr",           true,    2, false,    0, NULL },
	{ "strtod",           true,    2, false,    0, NULL },
	{ "strtok",           true,    2, false,    0, NULL },
	{ "strtol",           true,    3, false,    0, NULL },
	{ "strtoul",          true,    3, false,    0, NULL },
	{ "strxfrm",          true,    3, false,    0, NULL },
	{ "swprintf",         true,    0, false,    0, NULL }, // TODO
	{ "swscanf",          true,    0, false,    0, NULL }, // TODO
	{ "system",           true,    1, false,    0, NULL },
	{ "tmpfile",          true,    0, false,    0, NULL },
	{ "tmpnam",           true,    1, false,    0, NULL },
	{ "tolower",          true,    1, false,    0, NULL },
	{ "toupper",          true,    1, false,    0, NULL },
	{ "towlower",         true,    1, false,    0, NULL },
	{ "towupper",         true,    1, false,    0, NULL },
	{ "ungetc",           true,    2, false,    0, NULL },
	{ "ungetwc",          true,    2, false,    0, NULL },
	{ "vfprintf",         true,    0, false,    0, NULL }, // TODO
	{ "vfwprintf",        true,    0, false,    0, NULL }, // TODO
	{ "vprintf",          true,    0, false,    0, NULL }, // TODO
	{ "vsprintf",         true,    0, false,    0, NULL }, // TODO
	{ "vswprintf",        true,    0, false,    0, NULL }, // TODO
	{ "vwprintf",         true,    0, false,    0, NULL }, // TODO
	{ "wcrtomb",          true,    3, false,    0, NULL },
	{ "wcscat",           true,    2, false,    0, NULL },
	{ "wcschr",           true,    2, false,    0, NULL },
	{ "wcscmp",           true,    2, false,    0, NULL },
//	{ "wcscoll",          true,    0, false,    0, NULL }, // TODO
	{ "wcscpy",           true,    2, false,    0, NULL },
	{ "wcscspn",          true,    2, false,    0, NULL },
//	{ "wcsftime",         true,    0, false,    0, NULL }, // TODO
	{ "wcslen",           true,    1, false,    0, NULL },
	{ "wcsncat",          true,    3, false,    0, NULL },
	{ "wcsncmp",          true,    3, false,    0, NULL },
	{ "wcsncpy",          true,    3, false,    0, NULL },
//	{ "wcspbrk",          true,    0, false,    0, NULL }, // TODO
	{ "wcsrchr",          true,    2, false,    0, NULL },
	{ "wcsrtombs",        true,    4, false,    0, NULL },
	{ "wcsspn",           true,    2, false,    0, NULL },
	{ "wcsstr",           true,    2, false,    0, NULL },
//	{ "wcstod",           true,    0, false,    0, NULL }, // TODO
	{ "wcstok",           true,    3, false,    0, NULL },
//	{ "wcstol",           true,    0, false,    0, NULL }, // TODO
	{ "wcstombs",         true,    3, false,    0, NULL },
//	{ "wcstoul",          true,    0, false,    0, NULL }, // TODO
//	{ "wcsxfrm",          true,    0, false,    0, NULL }, // TODO
	{ "wctob",            true,    1, false,    0, NULL },
	{ "wctomb",           true,    2, false,    0, NULL },
	{ "wctype",           true,    1, false,    0, NULL },
	{ "wprintf",          true,    0, false,    0, NULL }, // TODO
	{ "wscanf",           true,    0, false,    0, NULL }, // TODO
	{ "_exit",           false,    1, false,    0, NULL },

	// end of array
	{ NULL,               false,   0, false,    0, NULL },
};
