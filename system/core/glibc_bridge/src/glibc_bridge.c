/* Initialize subsystems provided by Bionic that do not exist in glibc */

#include <stdio.h>
#include <sys/types.h>
#include <string.h>

int __system_properties_init(void);

void __attribute__ ((constructor)) glibc_bridge_init(void) {
    __system_properties_init();
}

#if !(HAVE_STRLCPY)
size_t
strlcpy(char *dst, const char *src, size_t size)
{
	const char *s = src;
	size_t n = size;

	if (n && --n)
		do {
			if (!(*dst++ = *src++))
				break;
		} while (--n);

	if (!n) {
		if (size)
			*dst = '\0';
		while (*src++);
	}

	return src - s - 1;
}
#endif
