#include <stdio.h>
#include <time.h>
#define fixmul(a,b) (a*b)>>16
/*
long fixmul(long a, long b)
{
	return (a*b)>>16;
}
*/
void intadd()
{
	int i1 = 0;
	int i2 = 0;
	for(int i=0;i<10000000;i++)
	{
		i1 += 1;
		i2 += i1;
		i1 += i2;
	}
}
void longadd()
{
	long l1 = 0;
	long l2 = 0;
	for(int i=0;i<10000000;i++)
	{
		l1 += 1;
		l2 += l1;
		l1 += l2;
	}
}
void floatadd()
{
	float f1 = 0.0f;
	float f2 = 0.0f;
	for(int i=0;i<10000000;i++)
	{
		f1 += 1.0f;
		f2 += f1;
		f1 += f2;
	}
}
void intmul()
{
	int i1 = 1;
	int i2 = 2;
	for(int i=0;i<10000000;i++)
	{
		i1 *= 1;
		i2 *= i1;
		i1 *= i2;
	}
}
void longmul()
{
	long l1 = 1;
	long l2 = 2;
	for(int i=0;i<10000000;i++)
	{
		l1 *= 1;
		l2 *= l1;
		l1 *= l2;
	}
}
void floatmul()
{
	float f1 = 1.1f;
	float f2 = 2.2f;
	for(int i=0;i<10000000;i++)
	{
		f1 *= 1.0f;
		f2 *= f1;
		f1 *= f2;
	}
}
void fixmul2()
{
	long f1 = 0x00ff00ff;
	long f2 = 0x0f000f00;
	for(int i=0;i<10000000;i++)
	{
		
		f1 = fixmul(f1,0x0f000f0f);
		f2 = fixmul(f1,f2);
		f1 = fixmul(f1,f2);
	}
}
int main()
{
	intadd();
	longadd();
	floatadd();

	intmul();
	longmul();
	floatmul();

	fixmul2();
	return 0;
}
