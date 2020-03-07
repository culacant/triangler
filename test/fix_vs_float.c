#include <stdio.h>
#include <stdlib.h>
// 32 bit arm tons of instructions, 64 bit arm few (something to do with 32 bit mull overflow)
// better with -O3
#include <stdint.h>

typedef int32_t fixed;

#define FX_FRAC 8

#define fix2float(a) (a / (float)(1<<FX_FRAC))
#define fix_make(a)  ((fixed)(a * (1<<FX_FRAC)))
#define fix_add(a,b) (a + b)
#define fix_sub(a,b) (a - b)
#define fix_mul(a,b) ((fixed)(((int64_t)a * b) >> FX_FRAC))
#define fix_div(a,b) ((fixed)(((int64_t)a << FX_FRAC) / b))


#define DATA_SIZE 100000000

//#define FLT_TEST
#define FIX_TEST

int main() {
#ifdef FLT_TEST
	float *data_fl = malloc(sizeof(float) * DATA_SIZE);
	data_fl[0] = 5.0f;

	for(int i=1;i<DATA_SIZE;i++){
		data_fl[i] = data_fl[i-1] +5.0f;
		data_fl[i] -= -15.0f;
		data_fl[i] = data_fl[i]/data_fl[i-1];
		data_fl[i] += data_fl[i-1]*data_fl[i];
	}
	printf("float: %f\n", data_fl[DATA_SIZE-1]);
#endif
#ifdef FIX_TEST
	fixed *data_fx = malloc(sizeof(fixed) * DATA_SIZE);
	data_fx[0] = fix_make(5.0f);

	for(int i=1;i<DATA_SIZE;i++){
		data_fx[i] = data_fx[i-1] + fix_make(5.0f);
		data_fx[i] += fix_make(15.0f);
		data_fx[i] = fix_div(data_fx[i],data_fx[i-1]);
		data_fx[i] += fix_mul(data_fx[i-1],data_fx[i]);
	}
	printf("fix: %f\n", fix2float(data_fx[DATA_SIZE-1]));
#endif

}
