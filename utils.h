#ifndef __UTILS_H
#define __UTILS_H

char **alloc_strcat(char **dest, char const *src);

char *utf8_next_char(char const * const p);
int utf8_strlen(char const *p);

#endif
