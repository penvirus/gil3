#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>

#include "common.h"

void *task(void *arg)
{
	void *handle = dlmopen(LM_ID_NEWLM, "/usr/lib/x86_64-linux-gnu/libpython2.7.so", RTLD_LAZY | RTLD_LOCAL);
	if (!handle) {
		ERR("failed to dlmopen: %s", dlerror());
		goto leave;
	}

	dlclose(handle);
leave:
	return NULL;
}

int main()
{
	int ret = 1;
	pthread_t t1;

	if (pthread_create(&t1, NULL, task, NULL)) {
		ERR("failed to pthread_create");
		goto leave;
	}
	pthread_join(t1, NULL);

	if (pthread_create(&t1, NULL, task, NULL)) {
		ERR("failed to pthread_create");
		goto leave;
	}
	pthread_join(t1, NULL);

	ret = 0;
leave:
	return ret;
}
