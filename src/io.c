/*
 *	io.c - Low-level i/o routines.
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

/*
 * some defines...
 */
#define	ISWHITE(c)	((c) == '\t' || (c) == ' ' || (c) == '\n')
#define ISCOMMENT(c)	((c)==';'  || (c)=='*')
#define	LSIZE		200	/* max. size of an input line */

/*
 * Tokens from the current line...
 */
char	buf_line[LSIZE];
long	line_no;

/*
 * readline() - read the next line from the file
 */
char *readline(fp)
	FILE	*fp;
	{
	register int 	len;

	/*
	 * Keep looping until we get a line of text
	 */
	for(;;)
		{
		ifn (fgets(buf_line,LSIZE,fp)) return NULL;
		++line_no;
		
		ifn(len = strlen(buf_line)) continue;
		if(buf_line[--len]=='\n') buf_line[len]='\0';

		return buf_line;
		}
	}

void tokenize(s, nb_hat, t_label, t_op, t_args, t_rest)
	register char	*s;
	int		*nb_hat;
	char		**t_label,**t_op,**t_args,**t_rest;
	{
	static	char	label[LSIZE], opcode[LSIZE], args[LSIZE];
	register char	*t;

	/*
	 * wipe out specials chars by cc -n
	 */
	*nb_hat = 0;
	while (*s=='^') {++s;++*nb_hat;}
	if (*s=='~' || *s=='|' || (!keepcomment && ISCOMMENT(*s)) || *s=='#')	
		{
		*label   = '\0';
		*t_label = 
		*t_op    =
		*t_args  =
		*t_rest  = label;
		return;
		}

	if(ISCOMMENT(*s)) 
		{
		*label   = '\0';
		*opcode  = '\0';
		*args    = '\0';
		goto end;
		}
	/*
	 * Grab the label, if any
	 */
	t = label;
	while(*s && !ISWHITE(*s) && *s!=':') *t++ = *s++;
	*t = '\0';

	while(*s==':' || ISWHITE(*s)) ++s;

	if(ISCOMMENT(*s))
		{
		*opcode  = '\0';
		*args    = '\0';
		goto end;
		}
	/*
	 * Grab the opcode
	 */
	t = opcode;
	while(*s && !ISWHITE(*s)) *t++ = *s++;
	*t = '\0';

	while(ISWHITE(*s)) ++s;

	if(ISCOMMENT(*s))
		{
		*args    = '\0';
		goto end;
		}
	/*
	 * Grab the arguments
	 */
	t = args;
	if     (*s=='\"') {while(*s) *t++ = *s++;}
	else if(*s=='\'') {while(*s) *t++ = *s++;}
	else 	{int p=0;
		while(*s && (!ISWHITE(*s) || p) && *s!=';')
			{	                     /* ; is the comment delimitor */ 
			if(*s=='(') ++p;
			else if(*s==')') --p;
			else if(*s=='[') ++p;
			else if(*s==']') --p;
			else if(*s=='\'') p=p?0:1;
			*t++ = *s++;
			}
		}
	*t = '\0';

	while(ISWHITE(*s)) ++s;

end:	*t_rest   = s;
	*t_label  = label;
	*t_op     = opcode;
	*t_args   = args;
	}
