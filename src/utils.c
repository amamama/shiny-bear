#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "utils.h"

void *aligned_alloc_and_null_check(size_t alignemnt, size_t size) {
	void *tmp = aligned_alloc(alignemnt, size);
	if (!tmp) {
		dbg_printf("aligned_alloc failed\n");
	}
	return tmp;
}

void *calloc_and_null_check(size_t nmemb, size_t size) {
	void *tmp = calloc(nmemb, size);
	if (!tmp) {
		dbg_printf("calloc failed\n");
	}
	return tmp;
}

void *malloc_and_null_check(size_t size) {
	void *tmp = malloc(size);
	if (!tmp) {
		dbg_printf("malloc failed\n");
	}
	return tmp;
}

void *realloc_and_null_check(void *ptr, size_t size) {
	void *tmp = realloc(ptr, size);
	if (!tmp) {
		dbg_printf("realloc failed\n");
	}
	return tmp;
}

char **alloc_strcat(char **dest, char const *src) {
	int destlen = 0, srclen = 0,wasdestnull = 0;
	destlen = (*dest)?strlen(*dest):0;
	srclen = src?strlen(src):0;
	wasdestnull = !(*dest);
	*dest = (char*)nc_realloc(*dest, (destlen + srclen + 1)*sizeof(char));
	if (wasdestnull) {
		memset(*dest, 0, strlen(src) + 1);
	}
	if (abs((intptr_t)*dest - (intptr_t)src) < strlen(*dest) + 1) {
		dbg_printf("warning: strncat takes restrict-qualified pointer, but two pointers may be same. it causes undefined behavior.\n%p and %p\n", *dest, src);
	}
	strncat(*dest, src, strlen(src));
	return dest;
}


char *utf8_next_char(char const * const p) {
	static char const utf8_skip_data[256] = {
		0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
		3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
	};
	return (char *)((p) + utf8_skip_data[*(unsigned char *)(p)]);
}

int utf8_strlen(char const *p) {
	int i;
	if (!p) return 0;
	for (i = 0; *p; i++) {
		p = utf8_next_char(p);
	}
	return i;
}

char *utf8_offset_to_pointer(char const *str, int offset) {
	if (offset < 0) {
		return NULL;
	}
	for(;offset--;) {
		str = utf8_next_char(str);
	}
	return (char *)str;
}

int utf8_pointer_to_offset(char const *str, char const * const pos) {
	int ret = 0;
	if (pos < str) {
		ret = - utf8_pointer_to_offset (pos, str);
	} else {
		for(; str < pos; ret++) {
			str = utf8_next_char(str);
		}
	}
	return ret;
}
