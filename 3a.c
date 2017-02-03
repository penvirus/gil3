#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int global = 23345678;

void print()
{
	printf("%ld: %d\n", syscall(SYS_gettid), global++);
}
