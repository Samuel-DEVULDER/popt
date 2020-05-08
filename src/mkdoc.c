/*
 *	mkdoc.c - This is a VERY simple program to extract comments
 *	in sourcefiles.
 */

#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

extern	void *malloc(long);

usage(n)
	char *n;
	{
	printf("Usage: %s [@<metafile>|<file1> <file2> ...]\n",n);
	exit(0);
	}

/*
 * Expand '\t' with ' '
 */
void preformat(buff)
	char *buff;
	{
	char *s,*t,buf[255];
	int i=0;
	strcpy(buf,buff);
	t=buff;s=buf;
	for(;*s;++s)
		{
		if(*s=='\t') do {*t++=' ';++i;} while(i&7);
		else if(*s!='\n') {*t++=*s;++i;}
		}
	*t=0;
	}

/*
 *	We look for line endinw with a  /* then
 *	the next line beginning with a "*". We dump that line
 *	until we find a line just containing * /.
 */
void process(n)
	char *n;
	{
	FILE *f;
	char buff[255];
	char *s,*t;
	int state=0;

	if(!(f=fopen(n,"r"))) {perror(n);return;}
	
	printf("\n**** WHAT IS DONE IN %s:\n\n",n);

	while(!feof(f))
		{
		if(!fgets(buff,250,f)) break;
		preformat(buff);
		s=buff;
		if(!*s) continue;
		while(s[1]) ++s;
		for(;s>buff && (*s==' ' || *s=='\t');--s);
		if(s==buff)  continue;
		if(s[-1]=='/' && s[0]=='*' && state==0) {state=1;continue;}
		for(s=buff;*s && (*s==' ' || *s=='\t'); ++s);
		if(*s=='*' && state==1)
			{
printf("\t+------------------------------------------------------------------\n");
			state = 2;
			}
		if(*s=='*' && s[1]!='/' && state==2)
			{
			if(strlen(s)<67) printf("\t|%s\n",s+1);
			else	{t=s+66;
				while(t>s && *t!=' ' && *t!='\t') --t;
				*t=0;
				printf("\t|%s\n",s+1);
				if(t[1]) printf("\t| %s\n",t+1);
				}
			}
		else state = 0;
		}
printf("\t+------------------------------------------------------------------\n");
	fclose(f);
	}

static void getmetafile(metafile,pac,pav)
	char *metafile;
	int *pac;
	char ***pav;
	{
	int ac=2,i=2;
	char **av=NULL;
	char line[80];
	FILE *f;
	if(f=fopen(metafile,"r"))
		{
		line[79]=0;
		while(fgets(line,79,f))
			{
			if(*line!=';' && *line!='#' && *line!=' ' && 
			   *line!='\n') ++ac;
			}
		rewind(f);
		if(!(av=malloc(ac*sizeof(*av)))) goto out_of_mem;
		while(fgets(line,79,f))
			{int l=strlen(line);
			if(l && line[l-1]=='\n') line[--l]='\0';
			if(*line==';' || *line=='#' || *line==' ' || !*line) 
				continue;
			if(!(av[i]=malloc(l+1)))
				{
out_of_mem:			printf("Out of mem !\n");
				exit(5);
				}
			strcpy(av[i],line);++i;
			}
		fclose(f);
		}
	else	{
		perror(metafile);
		exit(5);
		}
	*pac = ac;
	*pav = av;
	}

int main(argc,argv)
	int argc;
	char **argv;
	{
	char *revfile;
	FILE *f;
	int cctimes;
	int i;

	if(argc<=1) usage(argv[0]);
	if(*argv[1]=='@') getmetafile(argv[1]+1,&argc,&argv);

	printf("THIS IS A DUMP FILE THAT LISTS PEEPHOLE OPTIMIZATIONS DONE BY POPT,\n");
	printf("(c) by Samuel DEVULDER, 1995-97.\n\n");
	printf("UNSAFE optimizations are marked. Tell me if some marks are missing.\n");
	for(i=1;i<argc;++i) process(argv[i]);
	return 0;
	}
