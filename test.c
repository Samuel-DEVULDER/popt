long time();

typedef unsigned char ubyte;

test_palin(char *s)
	{
	char *d;
	for(d=s;*d++;);
	while(s<d && *--d == *s++);
	return (s==d);
	}

static unsigned char lastrand;

int char_rand()
	{
	unsigned ubyte last=lastrand*131+57;
	return lastrand=last;
	}

main()
	{
	long t=time(0);
	unsigned short lc1,lc2;
	int a1,a2,a3,a4,a5,a7;
	short a6;
	short *p=0,*q;
	a7=0;a6=1;
	for(lc1=1000;lc1--;)
		{
		printf("%d\n",char_rand());
		p=0;
		for(lc2=2000;lc2--;)
			{
			if(func1("123456789azertyuioppoiuytreza987654321")==1)
				p++;
			if(!(func1("111111")==1))
				{
				a1++;
				a6++;
				}
				
			a1=5;
			a2=a7;
			a3=5;
			a5=5;
			a6*=6;
			a7=*p++;
			}
		}
	printf("%d\n",time(0)-t);
	}
