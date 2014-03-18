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
