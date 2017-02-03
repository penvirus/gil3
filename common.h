#ifndef __COMMON_H__
#define __COMMON_H__

#define ERR(fmt, ...) fprintf(stderr, "[ERROR] " fmt " (%s:%d)\n", ## __VA_ARGS__, __FILE__, __LINE__)
#ifdef DEBUG
#define DBG(fmt, ...) fprintf(stderr, "[DEBUG] " fmt " (%s:%d)\n", ## __VA_ARGS__, __FILE__, __LINE__)
#else
#define DBG(fmt, ...)
#endif

#include <sys/time.h>
#include <stdio.h>

static inline void print_duration(struct timeval begin, struct timeval end)
{
	end.tv_usec -= begin.tv_usec;
	end.tv_sec -= begin.tv_sec;
	if (end.tv_usec < 0) {
		end.tv_usec += 1000000;
		--end.tv_sec;
	}

	printf("duration: %ld.%06ld seconds\n", end.tv_sec, end.tv_usec);
}

#endif
