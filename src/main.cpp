
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "rte_memcpy.h"

unsigned int gettime()
{
	static struct timezone tz={ 0,0 };
	struct timeval time;
	gettimeofday(&time,&tz);
	return (time.tv_sec * 1000 + time.tv_usec / 1000);
}

void sleepms(unsigned int millisec)
{
	usleep(millisec * 1000);
}

void benchmark(int dstalign, int srcalign, size_t size, int times)
{
	char *DATA1 = (char*)malloc(size + 64);
	char *DATA2 = (char*)malloc(size + 64);
	size_t LINEAR1 = ((size_t)DATA1);
	size_t LINEAR2 = ((size_t)DATA2);
	char *ALIGN1 = (char*)(((64 - (LINEAR1 & 63)) & 63) + LINEAR1);
	char *ALIGN2 = (char*)(((64 - (LINEAR2 & 63)) & 63) + LINEAR2);
	char *dst = (dstalign)? ALIGN1 : (ALIGN1 + 1);
	char *src = (srcalign)? ALIGN2 : (ALIGN2 + 3);
	unsigned int t1, t2;
	int k;
	
	sleepms(100);
	t1 = gettime();
	for (k = times; k > 0; k--) {
		memcpy(dst, src, size);
	}
	t1 = gettime() - t1;
	sleepms(100);
	t2 = gettime();
	for (k = times; k > 0; k--) {
		rte_memcpy(dst, src, size);
	}
	t2 = gettime() - t2;

	free(DATA1);
	free(DATA2);

	printf("result(dst %s, src %s): memcpy=%d ms, rte_memcpy=%d ms\n",
		dstalign? "aligned" : "unalign", 
		srcalign? "aligned" : "unalign", (int)t1, (int)t2);
}


void bench(int copysize, int times)
{
	printf("benchmark(size=%d bytes, times=%d):\n", copysize, times);
	benchmark(1, 1, copysize, times);
	benchmark(1, 0, copysize, times);
	benchmark(0, 1, copysize, times);
	benchmark(0, 0, copysize, times);
	printf("\n");
}


void random_bench(int maxsize, int times)
{
	static char A[11 * 1024 * 1024 + 2];
	static char B[11 * 1024 * 1024 + 2];
	static int random_offsets[0x10000];
	static int random_sizes[0x8000];
	unsigned int i, p1, p2;
	unsigned int t1, t2, t3;
	for (i = 0; i < 0x10000; i++) {	// generate random offsets
		random_offsets[i] = rand() % (10 * 1024 * 1024 + 1);
	}
	for (i = 0; i < 0x8000; i++) {	// generate random sizes
		random_sizes[i] = 1 + rand() % maxsize;
	}
	sleepms(100);
	t1 = gettime();
	for (p1 = 0, p2 = 0, i = 0; i < times; i++) {
		int offset1 = random_offsets[(p1++) & 0xffff];
		int offset2 = random_offsets[(p1++) & 0xffff];
		int size = random_sizes[(p2++) & 0x7fff];
		memcpy(A + offset1, B + offset2, size);
	}
	t1 = gettime() - t1;
	sleepms(100);
	t2 = gettime();
	for (p1 = 0, p2 = 0, i = 0; i < times; i++) {
		int offset1 = random_offsets[(p1++) & 0xffff];
		int offset2 = random_offsets[(p1++) & 0xffff];
		int size = random_sizes[(p2++) & 0x7fff];
		rte_memcpy(A + offset1, B + offset2, size);
	}
	t2 = gettime() - t2;

	printf("benchmark random access:\n");
	printf("memcpy=%dms, rte_memcpy=%dms\n\n", (int)t1, (int)t2);
}

int main(void)
{
	bench(32, 0x1000000);
	bench(64, 0x1000000);
	bench(512, 0x800000);
	bench(1024, 0x400000);
	bench(4096, 0x80000);
	bench(8192, 0x40000);
	bench(1024 * 1024 * 1, 0x800);
	bench(1024 * 1024 * 4, 0x200);
	bench(1024 * 1024 * 8, 0x100);
	
	bench(1024 * 1024 * 16, 0x80);
	bench(1024 * 1024 * 32, 0x40);
	bench(1024 * 1024 * 64, 0x20);
	bench(1024 * 1024 * 128, 0x10);
	bench(1024 * 1024 * 256, 0x8);

	random_bench(2048, 8000000);

	return 0;
}

