#include <pthread.h>
#include <Python.h>

#include "common.h"

void *task1(void *arg)
{
	PyRun_SimpleString(
		"import time\n"
		"print 'task1: {0:.6f}'.format(time.time())\n"
		"time.sleep(10)\n"
		"print 'task1: {0:.6f}'.format(time.time())\n"
		"print 'task1: {0}'.format(count)\n"
	);

	return NULL;
}

void *task2(void *arg)
{
	PyRun_SimpleString(
		"import time\n"
		"print 'task2: {0:.6f}'.format(time.time())\n"
		"print 'task2: {0}'.format(count)\n"
		"for i in xrange(23345678):\n"
		"    count += 1\n"
		"print 'task2: {0:.6f}'.format(time.time())\n"
	);

	return NULL;
}

int main()
{
	int ret = 1;
	pthread_t t1, t2;

	Py_Initialize();

	PyRun_SimpleString("count = 23345678");

	if (pthread_create(&t1, NULL, task1, NULL)) {
		ERR("failed to pthread_create");
		goto leave;
	}

	if (pthread_create(&t2, NULL, task2, NULL)) {
		ERR("failed to pthread_create");
		goto leave;
	}

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	ret = 0;
leave:
	Py_Finalize();
	return ret;
}
