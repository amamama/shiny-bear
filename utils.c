#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char **alloc_strcat(char **dest, char const *src) {
	#ifdef DEBUG
	puts(__func__);
	#endif

	int destlen = 0, srclen = 0,wasdestnull = 0;
	destlen = (*dest)?strlen(*dest):0;
	srclen = strlen(src);
	wasdestnull = !(*dest);
	*dest = (char*)realloc(*dest, (destlen + srclen + 1)*sizeof(char));
	if (dest) {
		if (wasdestnull) {
			memset(*dest, 0, strlen(src) + 1);
		}
		strncat(*dest, src, strlen(src));
	} else {
		fprintf(stderr,"realloc failed\n");
	}
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
