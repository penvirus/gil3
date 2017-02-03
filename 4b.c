#define _GNU_SOURCE
#include <dlfcn.h>

#include "common.h"

int main()
{
	int ret = 1;
	void *handle;

	handle = dlmopen(LM_ID_NEWLM, "/usr/lib/x86_64-linux-gnu/libpython2.7.so", RTLD_LAZY | RTLD_LOCAL);
	if (!handle) {
		ERR("failed to dlmopen: %s", dlerror());
		goto leave;
	}
	dlclose(handle);

	handle = dlmopen(LM_ID_NEWLM, "/usr/lib/x86_64-linux-gnu/libpython2.7.so", RTLD_LAZY | RTLD_LOCAL);
	if (!handle) {
		ERR("failed to dlmopen: %s", dlerror());
		goto leave;
	}
	dlclose(handle);

	ret = 0;
leave:
	return ret;
}
