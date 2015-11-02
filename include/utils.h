#ifndef __UTILS_H
#define __UTILS_H

#define free_and_assign(ptr) free(ptr),ptr = NULL
#ifdef DEBUG
	#define dbg_printf(...) (fprintf(stderr, __FILE__ ":%d:%s:", __LINE__, __func__), (fprintf(stderr, __VA_ARGS__)))
#else
	#define dbg_printf(...)
#endif

void *aligned_alloc_and_null_check(size_t, size_t);
void *calloc_and_null_check(size_t, size_t);
void *malloc_and_null_check(size_t);
void *realloc_and_null_check(void *, size_t);
#define nc_aligned_alloc (aligned_alloc_and_null_check)
#define nc_calloc (calloc_and_null_check)
#define nc_malloc (malloc_and_null_check)
#define nc_realloc (realloc_and_null_check)

char **alloc_strcat(char **dest, char const *src);

char *utf8_next_char(char const * const p);
int utf8_strlen(char const *p);
char *utf8_offset_to_pointer(char const *str, int offset);
int utf8_pointer_to_offset(char const *str, char const * const pos);

#endif
