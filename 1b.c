#include <pthread.h>
#include <Python.h>

#include "common.h"

void *task1(void *arg)
{
	PyGILState_STATE gstate;
	gstate = PyGILState_Ensure();

	PyRun_SimpleString(
		"import time\n"
		"print 'task1: {0:.6f}'.format(time.time())\n"
		"time.sleep(10)\n"
		"print 'task1: {0:.6f}'.format(time.time())\n"
		"print 'task1: {0}'.format(count)\n"
	);

	PyGILState_Release(gstate);

	return NULL;
}

void *task2(void *arg)
{
	PyGILState_STATE gstate;
	gstate = PyGILState_Ensure();

	PyRun_SimpleString(
		"import time\n"
		"print 'task2: {0:.6f}'.format(time.time())\n"
		"print 'task2: {0}'.format(count)\n"
		"for i in xrange(23345678):\n"
		"    count += 1\n"
		"print 'task2: {0:.6f}'.format(time.time())\n"
	);

	PyGILState_Release(gstate);

	return NULL;
}

int main()
{
	int ret = 1;
	pthread_t t1, t2;
	PyThreadState *th_state;

	Py_Initialize();
	PyEval_InitThreads();

	PyRun_SimpleString("count = 23345678");

	if (pthread_create(&t1, NULL, task1, NULL)) {
		ERR("failed to pthread_create");
		goto leave;
	}

	if (pthread_create(&t2, NULL, task2, NULL)) {
		ERR("failed to pthread_create");
		goto leave;
	}

	th_state = PyEval_SaveThread();
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	PyEval_RestoreThread(th_state);

	ret = 0;
leave:
	Py_Finalize();
	return ret;
}
