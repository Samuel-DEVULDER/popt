/*
 *	inst.c - Routines dealing with the parsing and output of 
 *		 instructions.
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

static	void	getarg	_((struct opnd *,char *));
static	int	isreg	_((char *));

/*
 * addinst(ip, line) - parse line and add an instruction after ip
 */
INST *addinst(ip, line)
	INST	*ip;
	char	*line;
	{
	register INST	*ni;
	register int	i;
	register char	*s,**optable;
	char		*arg2 = "";
	char		*label, *op, *args, *rest;
	int		nb_hat;

	ni = insinst(ip);

	/*
	 * put line number
	 */
	ni->lineno = line_no;

	/*
	 * Tokenize line
	 */
	tokenize(line, &nb_hat, &label, &op, &args, &rest);
	ni->nb_hat = nb_hat;

	/*
	 * put op to lowercase
	 */
	for(s = op; *s; *s = tolower(*s), ++s);

	/*
	 * Put label
	 */
	if(*label) ni->label = strsave(label);

	/*
	 * Find length
	 */
	for(s = op; *s && *s!='.'; ++s);
	if(*s == '.')   /* length specifier */
		{
		*s = '\0';
		switch(*++s)
			{
			case 'b':
			ni->flags |= LENB;
			break;

			case 'w':
			ni->flags |= LENW;
			break;

			case 'l':
			ni->flags |= LENL;
			break;

			case 's':   /* bsr.s ---> bsr */
			break;

			default:
			fprintf(stderr,	"%s: addinst() - Bad length spec '%c'\n",
				ProgName, *s);
			dumpline(NULL);
			exit(1);
			}
		}

	/*
	 * Find instruction
	 */
	for(optable = opnames, i = 0; *optable; ++optable, ++i)
		ifn (strcmp(op, *optable))
			{
			ni->opcode = ((i==DBRA)?DBF:i);
			break;
			}

	/*
	 * Process rest of line
	 */
	if (ni->opcode == NO_INST)
		{
		if(*op) {
			ifn(nowarning)
				{
				fprintf(stderr,	
				"%s: addinst() - Unknown opcode\n",ProgName);
				dumpline(NULL);
				}
			ni->opcode = UNKWN;
			ni->rest  = strsave(line+strlen(label));
			}
		else if(*rest) ni->rest = strsave(rest);
		return ni;
		}
	else if(*rest) ni->rest = strsave(rest);

	/*
	 * Look for the split between the first and second operands.
	 */
	for(s = args; *s ; ++s)
		{
		/*
		 * skip chars in parens, since an operand split can't
		 * occur within.
		 */
		if(*s == '(') while(*s != ')') ++s;
		if(*s == ',')
			{
			*s   = '\0';
			arg2 = ++s;
			break;
			}
		}

	getarg(&ni->src, args);
	getarg(&ni->dst, arg2);

	return ni;
	}

static bool fail;
/*
 * read a number. set fail.
 */
static int getnum(s)
	char *s;
	{
	bool neg = FALSE;
	bool hex = FALSE;
	int  num = 0;
	int  dig;

	fail = FALSE;
	if(*s=='-') {++s;neg = TRUE;}
	if(*s=='+') {++s;if(neg) goto failed;}
	if(*s=='$') {++s;if(!hex) hex = TRUE; else goto failed;}
	for(;*s;++s)
		{
		if(*s>='0' && *s<='9') dig = *s-'0';
		else if(hex && *s>='a' && *s<='f') dig = *s-'a'+10;
		else if(hex && *s>='A' && *s<='F') dig = *s-'A'+10;
		else goto failed;
		if(hex) num <<= 4;
		else	{
			num +=  num<<2;
			num <<= 1;
			}
		num +=  dig;
		}
	if(neg) num = -num;
	return num;
failed: fail = TRUE;
	return 0;
	}

/*
 * Rn-Rm | Rn
 */
static int dorspec(ps)
	register char	**ps;
	{
	char *s = *ps;
	register int	base;
	register int	m, n;
	register int	mask;

	fail = FALSE;
	base = (s[0] == 'd') || (s[0] == 'D') ? D0 :
	      ((s[0] == 'a') || (s[0] == 'A') ? A0 : -1);
	if(base == -1) goto failed; /* pas bon */

	m = *++s - '0' + base;
	if(*s>'7' || *s<'0') goto failed;

	if(*++s != '-') {*ps = s;return RM(m);}
	++s;
	n = (s[0] == 'd') || (s[0] == 'D') ? D0 :
	    ((s[0] == 'a') || (s[0] == 'A') ? A0 : -1);
	if(n!=base) goto failed;
	n = *++s - '0' + base;
	if(*s>'7' || *s<'0') goto failed;
	++s;
	for(mask=0; m <= n ;++m) mask |= RM(m);
	*ps = s;
	return mask;
failed:	fail = TRUE;
	return 0;
	}

/*
 * stomask() - convert a register list to a mask
 *
 * Convert a string like "Rm-Rn/Ro-Rp" or "Rm-Rn" to the appropriate
 * mask value.
 */
int stomask(s)
	char	*s;
	{
	register int	mask;

	mask = dorspec(&s);
	while(*s && !fail)
		{
		if(*s!='/') fail = TRUE;
		if(*++s) mask |= dorspec(&s);
		}
	return fail?0:mask;
	}

/*
 * getindex() - Parse & setup index mode/structure.
 */
static void getindex(op,s,regid,regidx)
	struct opnd *op;
	char *s;
	int regid,regidx;
	{
	int reg;
	fail = FALSE;

	if(*s==')')
		{
		if(*++s) goto failed;
		op->amode = regid;
		return;
		}
	if(*s!=',') goto failed;++s;
	if((reg=isreg(s))<0) goto failed;s+=2;
	ifn(reg>=FIRSTREG && reg<=LASTREG) goto failed;
	op->ireg  = reg;
	op->amode = regidx;
	if(*s=='.')
		{++s;
		if(tolower(*s)=='w') {}
		else if(tolower(*s)=='l') op->amode |= XLONG;
		else goto failed;
		++s;
		}
	if(*s=='*')
		{++s;
		if(!mc20 || !mc40)
			{
			ifn(nowarning)
				{
				fprintf(stderr,	
				"%s: getindex() - Warning ! 68020+ addressing mode found !\n",
				ProgName);
				dumpline(NULL);
				}
			mc20=TRUE;
			}
		if     (*s=='2') op->amode |= X2;
		else if(*s=='4') op->amode |= X4;
		else if(*s=='8') op->amode |= X8;
		else goto failed;
		++s;
		}
	if(*s!=')') goto failed;++s;
	if(*s) goto failed;
	return;
failed:	fail = TRUE;
	return;
	}

/*
 * getarg(op, s) - parse string 's' into the operand structure 'op'
 *
 * Hack alert!! The following code parses the operands only to the
 * extent needed by the optimizer. We're primarily interested in
 * details about addressing modes used, not in any expressions that
 * might be present. This code is highly tuned to the output of the
 * compiler.
 */
static void getarg(op, s)
	register struct opnd	*op;
	register char	*s;
	{
	register int	reg;
	register char	*p,*q;

	ifn(*s) {op->amode = NONE;return;}

	q=strsave(s);

	if(*s=='#') 
		{
		op->amode = IMM;
		op->disp  = getnum(++s);
		if(fail){
			op->amode |= SYMB;
			op->astr   = strsave(s);
			}
		goto ok;
		}

	if(*s=='\'' || *s=='\"')
		{
		op->amode = STRNG;
		op->astr  = strsave(s);
		goto ok;
		}

	reg = stomask(s);
	ifn(fail)
		{
		if(nb_bits(reg) == 1) 
			{
			op->amode = REG;
			op->areg  = isreg(s);
			}
		else	{
			op->amode = MSK;
			op->disp  = reg;
			}
		goto ok;
		}

	if(tolower(s[0])=='s' && tolower(s[1])=='p' && !s[2])
		{
		op->amode = REG;
		op->areg  = SP;
		goto ok;
		}

	op->disp = 0;

	if(s[0]=='-' && s[1]=='(')
		{
		op->amode = REGI|DEC;
		if((op->areg = isreg(s+2))<0) goto lastchance;
		if(s[4]!=')' || s[5]) goto lastchance;
		goto ok;
		}
		
	if(s[0]=='(' && ((op->areg=isreg(s+1))>=0) && s[3]==')')
		{
		if(s[4]=='+') op->amode = REGI|INC;
		else if(s[4]) goto lastchance;
		else op->amode = REGI;
		goto ok;
		}

	/* old syntax */
	
	for(p=s;*p && *p!='(';++p);	/* d16(An[,Dx[.W]]) */
					/* s = d16, p=An[,Dx[.W]] */
	if(*p=='(') *p++='\0';
	else	{
		op->amode = ABS;
		for(p=s;*p && *p!='.';++p);
		if(*p=='.')
			{
			*p++='\0';
			if(tolower(*p)=='w') op->amode|=XWORD;
			else if(tolower(*p)!='l' && *s) goto lastchance;
			else s=q;
			}

		op->disp = getnum(s);
		if(fail){
			op->amode |= SYMB;
			op->astr   = strsave(s);
			}
		goto ok;
		}

	if(tolower(p[0])=='p' && tolower(p[1])=='c')	
		{
		getindex(op,p+2,PCD,PCDX);
		goto ok2;
		}

	if((reg=isreg(p))>=A0 && (reg<=A7))
		{
		op->areg = reg;
		getindex(op,p+2,REGID,REGIDX);
ok2:		if(fail) goto lastchance;
		if(*s)	{
			op->disp = getnum(s);
			if(fail){
				op->amode |= SYMB;
				op->astr   = strsave(s);
				}
 			}
		goto ok;
		}
	if(*s) goto lastchance;

	/* new syntax */

	for(s=p;*p && *p!=',';++p);		/* (d16,An[,Dx[.W]]) */
						/* s = d16, p=An[,Dx[.W]] */
	if(*p==',') *p++='\0';
	else	{
		op->amode = ABS;
		for(p=s;*p && *p!=')';++p);
		if(*p!=')') goto lastchance;
		for(*p++='\0';*p && *p!='.';++p);
		if(*p=='.')
			{
			*p++='\0';
			if(tolower(*p)=='w') op->amode|=XWORD;
			else if(tolower(*p)!='l' && *s) goto lastchance;
			else s=q;
			}
		op->disp = getnum(s);
		if(fail){
			op->amode |= SYMB;
			op->astr   = strsave(s);
			}
		goto ok;
		}

	if(tolower(p[0])=='p' && tolower(p[1])=='c')	
		{
		getindex(op,p+2,PCD,PCDX);
		goto ok3;
		}

	if((reg=isreg(p))>=A0 && (reg<=A7))
		{
		op->areg = reg;
		getindex(op,p+2,REGID,REGIDX);
ok3:		if(fail) goto lastchance;
		if(*s)	{
			op->disp = getnum(s);
			if(fail){
				op->amode |= SYMB;
				op->astr   = strsave(s);
				}
 			}
		goto ok;
		}
	if(*s) goto lastchance;

ok:	free(q);
	return;
lastchance:
	ifn(nowarning)
		{
		fprintf(stderr, "%s: getarg() - Unknown addressing mode\n",
			ProgName);
		dumpline(NULL);
		}
	op->amode = STRNG;
	op->astr  = q;
	}	
/*
 * characters that can terminate a register name
 */
#define isterm(c) ((c) == '\0' || (c) == ')' || (c) == ',' || (c) == '.' || (c) == '*')

static int isreg(s)
	register char	*s;
	{
	if(tolower(s[0]) == 'd' && isdigit(s[1]) && isterm(s[2]))
		return D0 + (s[1] - '0');
	if(tolower(s[0]) == 'a' && isdigit(s[1]) && isterm(s[2]))
		return A0 + (s[1] - '0');
	if(tolower(s[0]) == 's' && tolower(s[1]) == 'p' && isterm(s[2]))
		return SP;
	return -1;
	}

/* 
 *	It's there in order to equilibrate inst.c and instbis.c in size
 */
static char *rstr(r)
	register char	r;
	{
	static	char	buf[3];

	if(r == SP)
		{
		buf[0] = 's';
		buf[1] = 'p';
		}
	else if (r >= A0 && r <= A6)
		{
		buf[0] = 'A';
		buf[1] = '0' + (r - A0);
		}
	else
		{
		buf[0] = 'D';
		buf[1] = '0' + (r - D0);
		}
	buf[2] = '\0';
	return buf;
	}

bool putop(s,op)
	char *s;
	register struct opnd *op;
	{
	switch (M(op->amode))
		{
		case NONE:
		*s='\0';
		break;

		case REG:
		sprintf(s, "%s", rstr(op->areg));
		break;
		
		case MSK:
		sprintf(s, "%s", masktos(op->disp));
		break;

		case IMM:
		if(op->amode & SYMB) sprintf(s, "#%s", op->astr);
		else sprintf(s, "#%ld", op->disp);
		break;

		case STRNG:
		sprintf(s, "%s", op->astr);
		break;

		case ABS:
/*		if(newsn)
			{
			if(op->amode==(ABS|SYMB)) sprintf(s, "%s", op->astr);
			else if(op->amode==(ABS|SYMB|XWORD)) 
				sprintf(s, "(%s)", op->astr);
			else sprintf(s, "(%ld)", op->disp);
			if(op->amode & XWORD) strcat(s,".W");
			}
		else	{	*/
			if(op->amode & SYMB) sprintf(s, "%s", op->astr);
			else sprintf(s, "%ld", op->disp);
			if(op->amode & XWORD) strcat(s,".W");
/*			}	*/
		break;

		case REGI:
		if(op->amode & DEC)
			sprintf(s, "-(%s)", rstr(op->areg));
		else if(op->amode & INC)
			sprintf(s, "(%s)+", rstr(op->areg));
		else	sprintf(s, "(%s)", rstr(op->areg));
		break;

		case REGID:
		if(newsn)
			{
			if(op->amode & SYMB) 
				sprintf(s, "(%s,%s)", op->astr,rstr(op->areg));
			else 	sprintf(s, "(%ld,%s)", op->disp, rstr(op->areg));
			}
		else	{
			if(op->amode & SYMB) 
				sprintf(s, "%s(%s)", op->astr,rstr(op->areg));
			else 	sprintf(s, "%ld(%s)", op->disp, rstr(op->areg));
			}
		break;

		case REGIDX:
		if(newsn)
			{
			if(op->amode & SYMB)	sprintf(s, "(%s,", op->astr);
			else 			sprintf(s, "(%ld,", op->disp);
			}
		else	{
			if(op->amode & SYMB)	sprintf(s, "%s(", op->astr);
			else 			sprintf(s, "%ld(", op->disp);
			}
		strcat(s,rstr(op->areg));
		sprintf(s+strlen(s),",%s.%c", rstr(op->ireg),
				    (op->amode & XLONG) ? 'l' : 'w');
		if(op->amode & X2) strcat(s,"*2");
		if(op->amode & X4) strcat(s,"*4");
		if(op->amode & X8) strcat(s,"*8");
		strcat(s,")");
		break;

		case PCD:
		if(newsn)
			{
			if(op->amode & SYMB) 
				sprintf(s, "(%s,pc)", op->astr);
			else 	sprintf(s, "(%ld,pc)", op->disp);
			}
		else	{
			if(op->amode & SYMB) 
				sprintf(s, "%s(pc)", op->astr);
			else 	sprintf(s, "%ld(pc)", op->disp);
			}
		break;

		case PCDX:
		if(newsn)
			{
			if(op->amode & SYMB)
				sprintf(s, "(%s,pc,%s.%c", op->astr, rstr(op->ireg),
					(op->amode & XLONG) ? 'l' : 'w');
			else	sprintf(s, "(%ld,pc,%s.%c", op->disp, rstr(op->ireg),
					(op->amode & XLONG) ? 'l' : 'w');
			}
		else	{
			if(op->amode & SYMB)
				sprintf(s, "%s(pc,%s.%c", op->astr, rstr(op->ireg),
					(op->amode & XLONG) ? 'l' : 'w');
			else	sprintf(s, "%ld(pc,%s.%c", op->disp, rstr(op->ireg),
					(op->amode & XLONG) ? 'l' : 'w');
			}
		if(op->amode & X2) strcat(s,"*2");
		if(op->amode & X4) strcat(s,"*4");
		if(op->amode & X8) strcat(s,"*8");
		strcat(s,")");
		break;

		default:
		fprintf(stderr, "%s: putop() - Bad addr. mode: %d\n",
			ProgName, op->amode);
		return FALSE;
		}
	return TRUE;
	}
