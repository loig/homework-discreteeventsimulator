#include <err.h>
#include <stdlib.h>

#include "util.h"

void *
xmalloc(size_t size)
{
	void *ptr;
	if ((ptr = malloc(size)) == NULL)
		err(1, NULL);
	else
		return ptr;
}
