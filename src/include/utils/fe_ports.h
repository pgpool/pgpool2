/*-------------------------------------------------------------------------
 *
 * fe_memutils.h
 *	  memory management support for frontend code
 *
 *-------------------------------------------------------------------------
 */

#ifndef POOL_PRIVATE
#error "This file is not expected to be compiled for pgpool utilities only"
#endif

#ifndef FE_PORTS
#define FE_PORTS

void *pg_malloc(size_t size);

void *pg_malloc0(size_t size);
void *pg_realloc(void *ptr, size_t size);
char *pg_strdup(const char *in);
void pg_free(void *ptr);
void *palloc(unsigned int size);
void *palloc0(unsigned int size);
void pfree(void *pointer);
char *pstrdup(const char *in);
void *repalloc(void *pointer, unsigned int size);

#endif