/*
 *	dumpline.c - displaying lines in case of error
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

/*
 *	dumpline(ip) - dump line in sourcefile represented by ip
 */
void dumpline(ip)
	INST *ip;
	{
	FILE *f;
	int  i,c;

	ifn(ip)	
		{
		fprintf(stderr, "File \"%s\":Line %ld:%s\n",\
				ifilename, line_no, buf_line);
		return;
		}
	ifn(i = ip->lineno) return;
	fprintf(stderr,"File \"%s\":Line %ld:", ifilename, i);
	if(f = fopen(ifilename, "r"))
		{
		--i;
		while(i && !feof(f))
			{
			c = fgetc(f);
			if(c == '\n') --i;
			}
		ifn(i)
			{
			while((c=fgetc(f))!=EOF && (c!='\n')) 
				fputc(c, stderr);
			fputc('\n',stderr);
			}
		fclose(f);
		}
	else perror(ifilename);
	}
