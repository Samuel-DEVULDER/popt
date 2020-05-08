/*
 *	progress.c - progress indicator
 *
 *	(c) by Samuel devulder
 */

#include <stdio.h>

void progress()
	{
	static char *str[]={
"\033[0 p\033[1m-\033[0m\010\033[ p",
"\033[0 p\\\033[0m\010\033[ p",
"\033[0 p|\033[0m\010\033[ p",
"\033[0 p/\033[0m\010\033[ p",
NULL};
	static char **s=str;

	printf(*s);fflush(stdout);
	if(!*++s) s=str;
	}
