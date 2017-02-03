#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <sched.h>

#include "common.h"

void *task(void *arg)
{
	void *handle = dlmopen(LM_ID_NEWLM, "./3a.so", RTLD_LAZY | RTLD_LOCAL);
	if (!handle) {
		ERR("failed to dlmopen: %s", dlerror());
		goto leave;
	}

	void (*print)() = dlsym(handle, "print");
	if (!print) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	print(); sched_yield();
	print(); sched_yield();
	print();
leave:
	if (handle)
		dlclose(handle);
	return NULL;
}

int main()
{
	int ret = 1;
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, task, NULL)) {
		ERR("failed to pthread_create");
		goto leave;
	}

	if (pthread_create(&t2, NULL, task, NULL)) {
		ERR("failed to pthread_create");
		goto leave;
	}

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	ret = 0;
leave:
	return ret;
}
