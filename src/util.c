/*
 *	util.c - safe memory allocation & safe file processing
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

char	*ifilename;

/*
 * safeopen - Open safely a file.
 */
FILE *safeopen(file, mode)
	char *file, *mode;
	{
	FILE *fp;
	ifn(fp = fopen(ifilename = file, mode))
		{
		fprintf(stderr, "%s: Can't open file %s\n", ProgName, file);
		exit(1);
		}
	line_no = 0;
	return	fp;
	}

/*
 * safeclose - Close it.
 */
void safeclose(fp)
	FILE	*fp;
	{
	fclose(fp);
	}

/*
 * strsave(s) - copy s to dynamically allocated space
 */
char *strsave(s)
	register char	*s;
	{
	return strcpy(alloc((unsigned) (strlen(s) + 1)), s);
	}

/*
 * alloc() - malloc with error checking
 */
char *alloc(n)
	int	n;
	{
	char	*p;

	ifn(p = malloc(n))
		{
		fprintf(stderr,"%s: Out of memory (%d byte(s) required)\n",
			ProgName, n);
		exit(1);
		}
	return p;
	}
