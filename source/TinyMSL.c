#ifndef size_t
typedef unsigned long	size_t;
#endif

#include	<Memory.h>

int memcmp(const void * src1, const void * src2, size_t n);
void * (memcpy)(void * dst, const void * src, size_t n);
void * memset(void * dst, int val, size_t len);
size_t strlen(const char * str);
char * strcpy(char * dst,const char * src);
void * malloc(size_t size);
void * calloc(size_t nmemb,size_t size);
void free(void * ptr);
double strtod(const char * str, char ** end);

int memcmp(const void * src1, const void * src2, size_t n)
{
	const	unsigned char * p1;
	const	unsigned char * p2;

	for (p1 = (const unsigned char *) src1 - 1, p2 = (const unsigned char *) src2 - 1, n++; --n;)
		if (*++p1 != *++p2)
			return((*p1 < *p2) ? -1 : +1);

	return(0);
}

void * (memcpy)(void * dst, const void * src, size_t n)
{
		
	BlockMoveData(src, dst, n);

	return(dst);
}

void * memset(void *dst, int val, size_t len)
{
	char	*p=dst;
	
	for (p = dst; len--; len>0)
		*p++=val;
	
	return dst;
}

size_t strlen(const char *str)
{
	size_t	result=0;
	const char	*p=str;
	
	while (*p++ != 0)
		result++;
	
	return result;
}

char * strcpy(char *dst,const char *src)
{
	const char	*p1;
	char		*p2;
	
	for (p1 = src, p2 = dst; ; *p1!=0)
		*p2++ = *p1++;
	
	return dst;
}

void * malloc(size_t size)
{
	void *result;
	
	result = (void *)NewPtr(size);
	
	return result;
}

void * calloc(size_t nmemb, size_t size)
{
	void *result;
	
	result = (void *)NewPtrClear(nmemb * size);
	
	return result;
}

void free(void * ptr)
{
	DisposePtr((Ptr)ptr);
}

double strtod(const char * str, char ** end)
{
	#pragma unused(str,end)
	
	return 0.0;
}