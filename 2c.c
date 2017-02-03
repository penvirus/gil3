#define _GNU_SOURCE
#include <pthread.h>
#include <dlfcn.h>

#include "common.h"

void *task1(void *arg)
{
	void *handle = dlmopen(LM_ID_NEWLM, "/usr/lib/x86_64-linux-gnu/libpython2.7.so", RTLD_LAZY | RTLD_LOCAL);
	if (!handle) {
		ERR("failed to dlmopen: %s", dlerror());
		goto leave;
	}

	void (*_Py_Initialize)() = dlsym(handle, "Py_Initialize");
	if (!_Py_Initialize) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	int (*_PyRun_SimpleString)(const char *) = dlsym(handle, "PyRun_SimpleString");
	if (!_PyRun_SimpleString) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	void (*_Py_Finalize)() = dlsym(handle, "Py_Finalize");
	if (!_Py_Finalize) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	_Py_Initialize();
	_PyRun_SimpleString("count = 23345678");
	_PyRun_SimpleString(
		"import time\n"
		"print 'task1: {0:.6f}'.format(time.time())\n"
		"time.sleep(10)\n"
		"print 'task1: {0:.6f}'.format(time.time())\n"
		"print 'task1: {0}'.format(count)\n"
	);
	_Py_Finalize();

leave:
	if (handle)
		dlclose(handle);
	return NULL;
}

void *task2(void *arg)
{
	void *handle = dlmopen(LM_ID_NEWLM, "/usr/lib/x86_64-linux-gnu/libpython2.7.so", RTLD_LAZY | RTLD_LOCAL);
	if (!handle) {
		ERR("failed to dlmopen: %s", dlerror());
		goto leave;
	}

	void (*_Py_Initialize)() = dlsym(handle, "Py_Initialize");
	if (!_Py_Initialize) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	int (*_PyRun_SimpleString)(const char *) = dlsym(handle, "PyRun_SimpleString");
	if (!_PyRun_SimpleString) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	void (*_Py_Finalize)() = dlsym(handle, "Py_Finalize");
	if (!_Py_Finalize) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	_Py_Initialize();
	_PyRun_SimpleString("count = 23345678");
	_PyRun_SimpleString(
		"import time\n"
		"print 'task2: {0:.6f}'.format(time.time())\n"
		"print 'task2: {0}'.format(count)\n"
		"for i in xrange(23345678):\n"
		"    count += 1\n"
		"print 'task2: {0:.6f}'.format(time.time())\n"
	);
	_Py_Finalize();

leave:
	if (handle)
		dlclose(handle);
	return NULL;
}

int main()
{
	int ret = 1;
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, task1, NULL) != 0) {
		ERR("failed to pthread_create");
		goto leave;
	}

	if (pthread_create(&t2, NULL, task2, NULL) != 0) {
		ERR("failed to pthread_create");
		goto leave;
	}

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	ret = 0;
leave:
	return ret;
}
