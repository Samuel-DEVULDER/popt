/*
 *	mkrev.c
 */
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

void usage(char *n)
	{
	printf("Usage: %s <rev.c> [@<linkfile>|<file1> <file2> ...]\n",n);
	printf("\nUpdate the file <rev.c> with <linkfile> or <file1> <file2> ...\n");
	printf("See the content of <rev.c> for more infos.\n");
	exit(0);
	}

static char VNAME[256];

int getcctimes(char *n)
	{
	FILE	*f;
	char	s[20];
	long	cctimes;
	
	if(f=fopen(n,"r"))
		{
		int c;
		char *t;

		fscanf(f,"%10s",s);
		if(strcmp(s,"#define")) goto err;
		fscanf(f,"%10s",s);
		if(strcmp(s,"cctimes")) goto err;
		if(fscanf(f,"%ld",&cctimes)!=1) goto err;

		fscanf(f,"%10s",s);
		if(strcmp(s,"#define")) goto err;
		fscanf(f,"%10s",s);
		if(strcmp(s,"VNAME")) goto err;
		t=VNAME;
		while(1) 
			{
			c=fgetc(f);
			if(c==EOF || c=='\n') goto err;
			if(c=='"') break;
			}
		while(1) 
			{
			c=fgetc(f);
			if(c==EOF || c=='\n') goto err;
			if(c=='"') break;
			*t++=(char)c;
			}
		*t='\0';
		
		fclose(f);
		return cctimes;
		}
	else
		{
		perror(n);
		goto err2;
		}
	err:
	printf("%s: bad revision file\n",n);
	err2:
	printf("May be create a file called \"%s\" whose first lines are:\n",n);
	printf("#define cctimes 0\n");
	printf("#define VNAME \"program name\"\n");
	printf("could help.\n");
	exit(5);
	}

void initrevfile(FILE *f,int cctimes)
	{
	time_t t;
	struct tm *tm;

	time(&t);
	tm=localtime(&t);
	fprintf(f,"#define cctimes %d\n",cctimes);
	fprintf(f,"#define VNAME \"%s\"\n\n",VNAME);
	fprintf(f,"#include <stdio.h>\n");
	fprintf(f,"#include <string.h>\n");
	fprintf(f,"#include <stdlib.h>\n\n");
	fprintf(f,"#ifndef VNUM\n");
	fprintf(f,"#define VNUM \"0.0\"\n");
	fprintf(f,"#endif\n\n");
	fprintf(f,"#ifdef __PDC__\n");
	fprintf(f,"char IDstring[1]={0};\n");
	fprintf(f,"static char IDstring1[]=\"$VER: %s \"VNUM\" (%02d.%02d.%d) [c: %d] © by Samuel Devulder.\";\n",
		VNAME,tm->tm_mday,tm->tm_mon+1,tm->tm_year,cctimes);
	fprintf(f,"#else\n");
	fprintf(f,"char IDstring[]=\"\\0$VER: %s \"VNUM\" (%02d.%02d.%d) [c: %d] © by Samuel Devulder.\";\n",
		VNAME,tm->tm_mday,tm->tm_mon+1,tm->tm_year,cctimes);
	fprintf(f,"#endif\n\n");
	fprintf(f,"static void _revinfo(char *name)\n");
	fprintf(f,"\t{\n");
	fprintf(f,"\tprintf(\"%%s\\n\\n\",((char*)IDstring)+1);\n");
	fprintf(f,"\tprintf(\"revinfo(\\\"%%s\\\"): ");
	fprintf(f,"%d compilation",cctimes);
	if(cctimes>1) fprintf(f,"s");
	fprintf(f,"\\n\",name);\n");
	}

void closrevfile(FILE *f)
	{
	fprintf(f,"\t}\n\n");
	fprintf(f,"void revinfo(int argc, char **argv)\n");
	fprintf(f,"\t{\n\tregister int i;\n\tfor(i=1;i<argc;++i)\n");
	fprintf(f,"\t\tif(!strcmp(argv[i],\"-revinfo\"))\n");
	fprintf(f,"\t\t\t{_revinfo(argv[0]);exit(0);}\n");
	fprintf(f,"\t}\n");
	}

void midrevfile(FILE *f,char *n)
	{
	struct stat st;
	int pos;
	char *s,*t;

	if(stat(n,&st)) {perror(n);return;}
	fprintf(f,"\tputs(\"    ");
	pos = 4;
	fprintf(f,n);pos+=strlen(n);
	if(pos>=40) {fprintf(f,"\\n");pos=0;}
	while(pos<40) {pos+=8;pos&=~7;fprintf(f,"\\t");}
/*	  fprintf(f,!(st.st_mode & S_IREAD)?    "-":"r");
	fprintf(f,!(st.st_mode & S_IWRITE)?   "-":"w");
	fprintf(f,!(st.st_mode & S_IEXEC)?    "-":"e"); */
	fprintf(f,"%8ld  ",st.st_size);
	t=s=ctime(&st.st_mtime);
	while(*t!='\n') ++t;*t='\0';
	fprintf(f,s);
	fprintf(f,"\");\n");
	}

static int cmpfunc(a,b)
	char **a,**b;
	{
	return strcmp(*a,*b);
	}

static void getlinkfile(linkfile,pac,pav)
	char *linkfile;
	int *pac;
	char ***pav;
	{
	int ac=2,i=2;
	char **av=NULL;
	char line[80];
	FILE *f;
	if(f=fopen(linkfile,"r"))
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
		perror(linkfile);
		exit(5);
		}
	*pac = ac;
	*pav = av;
	}

main(int argc,char **argv)
	{
	FILE *f;
	int cctimes;
	int i;
	char *revfile;

	if(argc<=1) usage(argv[0]);
	cctimes=getcctimes(revfile=argv[1]);
	++cctimes;
	if(*argv[2]=='@') getlinkfile(argv[2]+1,&argc,&argv);
	if(f=fopen(revfile,"w"))
		{
		initrevfile(f,cctimes);
		qsort(argv+2,argc-2,sizeof(*argv),(void*)cmpfunc);
		for(i=2;i<argc;++i) midrevfile(f,argv[i]);
		closrevfile(f);
		fclose(f);
		}
	else
		{
		perror(revfile);
		exit(5);
		}
	return 0;
	}
