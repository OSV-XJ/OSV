#include <inc/types.h>
#include <stdarg.h>
#include <stddef.h>

int memcmp(const void *v1, const void *v2, size_t n)
{
	if(n != 0)
	{
		const unsigned char *p1 = v1, *p2 = v2;
		for(;n > 0; n --)
		{
			if(*p1 ++ != *p2 ++)
				return (*--p1 - * --p2);
		}
	}
	return 0;
}
