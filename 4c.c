#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>

#include "common.h"

void *task(void *arg)
{
	void *handle = arg;

	void (*_Py_Initialize)() = dlsym(handle, "Py_Initialize");
	if (!_Py_Initialize) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}
	_Py_Initialize();
leave:
	return NULL;
}

int main()
{
	int ret = 1;
	pthread_t t1, t2;
	void *h1 = NULL, *h2 = NULL;

	h1 = dlmopen(LM_ID_NEWLM, "/usr/lib/x86_64-linux-gnu/libpython2.7.so", RTLD_LAZY | RTLD_LOCAL);
	if (!h1) {
		ERR("failed to dlmopen: %s", dlerror());
		goto leave;
	}
	h2 = dlmopen(LM_ID_NEWLM, "/usr/lib/x86_64-linux-gnu/libpython2.7.so", RTLD_LAZY | RTLD_LOCAL);
	if (!h2) {
		ERR("failed to dlmopen: %s", dlerror());
		goto leave;
	}

	if (pthread_create(&t1, NULL, task, h1)) {
		ERR("failed to pthread_create");
		goto leave;
	}
	if (pthread_create(&t2, NULL, task, h2)) {
		ERR("failed to pthread_create");
		goto leave;
	}

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	ret = 0;
leave:
	if (h1)
		dlclose(h1);
	if (h2)
		dlclose(h2);
	return ret;
}
