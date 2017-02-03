#include <dlfcn.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <Python.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#include "common.h"
#define CONFIG_FILE "./config.json"

struct common_operations {
	void (*_Py_InitializeEx)(int);
	void (*_Py_Finalize)();
	int (*_PyRun_SimpleFileEx)(FILE *, const char *, int);
	PyObject *(*_PyModule_New)(const char *);
	PyObject *(*_PyModule_GetDict)(PyObject *);
	PyObject *(*_PyDict_GetItemString)(PyObject *, const char *);
	PyObject *(*_PyImport_ImportModule)(const char *);
	PyObject *(*_PyObject_CallObject)(PyObject *, PyObject *);
	void (*_Py_IncRef)(PyObject *);
	void (*_Py_DecRef)(PyObject *);
	void (*_PyEval_InitThreads)();
	PyThreadState *(*_PyEval_SaveThread)();
	void (*_PyEval_RestoreThread)(PyThreadState *);
	void *(*_PyGILState_Ensure)();
	void (*_PyGILState_Release)(void *);
	int (*_PyRun_SimpleString)(const char *);
	long (*_PyInt_AsLong)(PyObject *);
	PyObject *(*_PyBool_FromLong)(long);
	int (*_PyDict_SetItemString)(PyObject *, const char *, PyObject *);
};

struct context {
	pthread_mutex_t lock;
	pthread_cond_t cond;
	PyObject *calc_main_dict;
	struct common_operations *calc_ops;
	int cur_switch_on;
};

static int resolve_common_operations(void *handle, struct common_operations *ops)
{
	int ret = 0;

	ops->_Py_InitializeEx = dlsym(handle, "Py_InitializeEx");
	if (!ops->_Py_InitializeEx) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_Py_Finalize = dlsym(handle, "Py_Finalize");
	if (!ops->_Py_Finalize) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_PyRun_SimpleFileEx = dlsym(handle, "PyRun_SimpleFileEx");
	if (!ops->_PyRun_SimpleFileEx) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_PyModule_New = dlsym(handle, "PyModule_New");
	if (!ops->_PyModule_New) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_PyModule_GetDict = dlsym(handle, "PyModule_GetDict");
	if (!ops->_PyModule_GetDict) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_PyDict_GetItemString = dlsym(handle, "PyDict_GetItemString");
	if (!ops->_PyDict_GetItemString) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_PyImport_ImportModule = dlsym(handle, "PyImport_ImportModule");
	if (!ops->_PyImport_ImportModule) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_PyObject_CallObject = dlsym(handle, "PyObject_CallObject");
	if (!ops->_PyObject_CallObject) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_Py_IncRef = dlsym(handle, "Py_IncRef");
	if (!ops->_Py_IncRef) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_Py_DecRef = dlsym(handle, "Py_DecRef");
	if (!ops->_Py_DecRef) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_PyEval_InitThreads = dlsym(handle, "PyEval_InitThreads");
	if (!ops->_PyEval_InitThreads) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_PyEval_SaveThread = dlsym(handle, "PyEval_SaveThread");
	if (!ops->_PyEval_SaveThread) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_PyEval_RestoreThread = dlsym(handle, "PyEval_RestoreThread");
	if (!ops->_PyEval_RestoreThread) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_PyGILState_Ensure = dlsym(handle, "PyGILState_Ensure");
	if (!ops->_PyGILState_Ensure) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_PyGILState_Release = dlsym(handle, "PyGILState_Release");
	if (!ops->_PyGILState_Release) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_PyRun_SimpleString = dlsym(handle, "PyRun_SimpleString");
	if (!ops->_PyRun_SimpleString) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_PyInt_AsLong = dlsym(handle, "PyInt_AsLong");
	if (!ops->_PyInt_AsLong) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_PyBool_FromLong = dlsym(handle, "PyBool_FromLong");
	if (!ops->_PyBool_FromLong) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ops->_PyDict_SetItemString = dlsym(handle, "PyDict_SetItemString");
	if (!ops->_PyDict_SetItemString) {
		ERR("failed to dlsym: %s", dlerror());
		goto leave;
	}

	ret = 1;
leave:
	return ret;
}

void *calc_task(void *arg)
{
	struct context *ctx = arg;
	struct common_operations ops = {0};
	void *handle = NULL;
	FILE *fp = NULL;
	PyObject *main_module = NULL, *run_method = NULL, *switch_on;

	system("cp /usr/lib/x86_64-linux-gnu/libpython2.7.so calc_libpython2.7.so");

	handle = dlopen("./calc_libpython2.7.so", RTLD_LAZY | RTLD_LOCAL);
	if (!handle) {
		ERR("failed to dlopen: %s", dlerror());
		goto leave;
	}

	if (!resolve_common_operations(handle, &ops)) {
		ERR("failed to resolve_common_operations");
		goto leave;
	}

	fp = fopen("6a.py", "r");
	if (!fp) {
		ERR("failed to fopen: %s", strerror(errno));
		goto leave;
	}

	ops._PyEval_InitThreads();
	ops._Py_InitializeEx(0);
	ops._PyRun_SimpleFileEx(fp, "6a.py", 1 /* closeit, fp will be closed */);

	main_module = ops._PyImport_ImportModule("__main__");
	if (!main_module) {
		ERR("failed to _PyImport_ImportModule");
		goto leave_python;
	}

	ctx->calc_main_dict = ops._PyModule_GetDict(main_module);
	if (!ctx->calc_main_dict) {
		ERR("failed to _PyModule_GetDict");
		goto leave_python;
	}
	ops._Py_IncRef(ctx->calc_main_dict);

	switch_on = ops._PyDict_GetItemString(ctx->calc_main_dict, "switch_on");
	if (!switch_on) {
		ERR("failed to _PyDict_GetItemString");
		goto leave_python;
	}
	ctx->cur_switch_on = ops._PyInt_AsLong(switch_on);

	run_method = ops._PyDict_GetItemString(ctx->calc_main_dict, "run");
	if (!run_method) {
		ERR("failed to _PyDict_GetItemString");
		goto leave_python;
	}
	ops._Py_IncRef(run_method);

	pthread_mutex_lock(&ctx->lock);
	ctx->calc_ops = &ops;
	pthread_cond_signal(&ctx->cond);
	pthread_mutex_unlock(&ctx->lock);

	ops._PyObject_CallObject(run_method, NULL);

	/* never reachable */

leave_python:
	if (run_method)
		ops._Py_DecRef(run_method);
	if (ctx->calc_main_dict)
		ops._Py_DecRef(ctx->calc_main_dict);
	if (main_module);
		ops._Py_DecRef(main_module);
	ops._Py_Finalize();

leave:
	if (handle)
		dlclose(handle);
	system("rm -f calc_libpython2.7.so");
	return NULL;
}

void *config_task(void *arg)
{
	struct context *ctx = arg;
	struct common_operations ops = {0};
	void *handle, *state;
	PyObject *main_module = NULL, *main_dict, *calc, *obj;
	int sig;
	sigset_t sigmask;

	system("cp /usr/lib/x86_64-linux-gnu/libpython2.7.so config_libpython2.7.so");

	handle = dlopen("./config_libpython2.7.so", RTLD_LAZY | RTLD_LOCAL);
	if (!handle) {
		ERR("failed to dlopen: %s", dlerror());
		goto leave;
	}

	if (!resolve_common_operations(handle, &ops)) {
		ERR("failed to resolve_common_operations");
		goto leave;
	}

	ops._Py_InitializeEx(0);

	main_module = ops._PyImport_ImportModule("__main__");
	if (!main_module) {
		ERR("failed to _PyImport_ImportModule");
		goto leave_python;
	}

	main_dict = ops._PyModule_GetDict(main_module);
	if (!main_dict) {
		ERR("failed to _PyModule_GetDict");
		goto leave_python;
	}

	pthread_mutex_lock(&ctx->lock);
	if (!ctx->calc_ops)
		pthread_cond_wait(&ctx->cond, &ctx->lock);
	pthread_mutex_unlock(&ctx->lock);

	while (1) {
		ops._PyRun_SimpleString(
			"import json\n"
			"calc = json.load(open('"CONFIG_FILE"'))['calc']\n"
		);

		calc = ops._PyDict_GetItemString(main_dict, "calc");
		if (!calc) {
			ERR("failed to _PyDict_GetItemString");
			goto leave_python;
		}

		if (ops._PyInt_AsLong(calc) != ctx->cur_switch_on) {
			DBG("state changed: from %d to %d", ctx->cur_switch_on, 1 - ctx->cur_switch_on);

			ctx->cur_switch_on = 1 - ctx->cur_switch_on;

			state = ctx->calc_ops->_PyGILState_Ensure();
			obj = ctx->calc_ops->_PyBool_FromLong(ctx->cur_switch_on);
			if (!obj) {
				ERR("failed to _PyBool_FromLong");
				ctx->calc_ops->_PyGILState_Release(state);
				goto leave_python;
			}
			if (ctx->calc_ops->_PyDict_SetItemString(ctx->calc_main_dict, "switch_on", obj)) {
				ERR("failed to _PyDict_SetItemString");
				ctx->calc_ops->_Py_DecRef(obj);
				ctx->calc_ops->_PyGILState_Release(state);
				goto leave_python;
			}
			ctx->calc_ops->_Py_DecRef(obj);
			ctx->calc_ops->_PyGILState_Release(state);
		}

		sigemptyset(&sigmask);
		sigaddset(&sigmask, SIGUSR1);
		sigwait(&sigmask, &sig);
	}

leave_python:
	if (main_module);
		ops._Py_DecRef(main_module);
	ops._Py_Finalize();

	/* never reachable */
leave:
	system("rm -f config_libpython2.7.so");
	return NULL;
}

void *nihao5884_task(void *arg)
{
	struct common_operations ops = {0};
	void *handle = NULL;
	int sfd = -1, cfd = -1;
	struct sockaddr_in addr;

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1) {
		ERR("failed to socket: %s", strerror(errno));
		goto leave;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(5884);
	if (inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr) != 1) {
		ERR("failed to inet_pton");
		goto leave;
	}

	if (bind(sfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		ERR("failed to bind: %s", strerror(errno));
		goto leave;
	}

	if (listen(sfd, 1) == -1) {
		ERR("failed to listen: %s", strerror(errno));
		goto leave;
	}

	system("cp /usr/lib/x86_64-linux-gnu/libpython2.7.so nihao5884_libpython2.7.so");

	handle = dlopen("./nihao5884_libpython2.7.so", RTLD_LAZY | RTLD_LOCAL);
	if (!handle) {
		ERR("failed to dlopen: %s", dlerror());
		goto leave;
	}

	if (!resolve_common_operations(handle, &ops)) {
		ERR("failed to resolve_common_operations");
		goto leave;
	}

	ops._Py_InitializeEx(0);
	while (1) {
		static const char *success = "{\"success\": true}";
		static const char *fail = "{\"success\": false}";
		struct sockaddr_in caddr;
		socklen_t len = sizeof(caddr);
		int n;
		char buf[40960];

		cfd = accept(sfd, (struct sockaddr *) &caddr, &len);
		if (cfd == -1) {
			ERR("failed to accept: %s", strerror(errno));
			goto python_leave;
		}
		DBG("python code from %s:%d", inet_ntop(AF_INET, &caddr.sin_addr, buf, len), ntohs(caddr.sin_port));

		n = read(cfd, buf, sizeof(buf) / sizeof(buf[0]));
		if (n == -1) {
			ERR("failed to read: %s", strerror(errno));
			goto python_leave;
		}
		buf[n] = 0;

		/* forget it if write isn't success */
		if (ops._PyRun_SimpleString(buf) == 0)
			write(cfd, success, strlen(success));
		else
			write(cfd, fail, strlen(fail));

		close(cfd);
		cfd = -1;
	}

python_leave:
	ops._Py_Finalize();

leave:
	if (cfd != -1)
		close(cfd);
	if (sfd != -1)
		close(sfd);
	if (handle)
		dlclose(handle);
	system("rm -f nihao5884_libpython2.7.so");
	return NULL;
}

int main()
{
	int ret = 1;
	pthread_t calc, config, nihao5884;
	struct context ctx = {0};
	sigset_t sigmask;

	pthread_mutex_init(&ctx.lock, NULL);
	pthread_cond_init(&ctx.cond, NULL);

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGUSR1);
	if (pthread_sigmask(SIG_BLOCK, &sigmask, NULL)) {
		ERR("failed to pthread_sigmask");
		goto leave;
	}

	if (pthread_create(&calc, NULL, calc_task, &ctx)) {
		ERR("failed to pthread_create");
		goto leave;
	}

	if (pthread_create(&config, NULL, config_task, &ctx)) {
		ERR("failed to pthread_create");
		goto leave;
	}

	if (pthread_create(&nihao5884, NULL, nihao5884_task, &ctx)) {
		ERR("failed to pthread_create");
		goto leave;
	}

	pthread_join(calc, NULL);
	pthread_join(config, NULL);
	pthread_join(nihao5884, NULL);

	/* never reachable */

	ret = 0;
leave:
	pthread_cond_destroy(&ctx.cond);
	pthread_mutex_destroy(&ctx.lock);
	return ret;
}
