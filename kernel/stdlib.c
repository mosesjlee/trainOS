
#include <kernel.h>


int k_strlen(const char* str)
{
	int l = 0;
	while (*str++ != '\0') ++l;
	return (l);
}

void* k_memcpy(void* dst, const void* src, int len)
{
	char* cdst = (char*) dst;
	char* csrc = (char*) src;
	while (len > 0) {
		*cdst++ = *csrc++;
		len--;
	}
	return (dst);
}

int k_memcmp(const void* b1, const void* b2, int len)
{
	unsigned char* c1 = (unsigned char*) b1;
	unsigned char* c2 = (unsigned char*) b2;
	while (len > 0) {
		int d = *c1++ - *c2++;
		if (d != 0) return (d);
		len--;
	}

	return (0);
}

//My own stdlib functions
int k_strcmp(const char * s1, const char *s2)
{
   while(*s1 != '\0' && *s2 != '\0')
   {
      int d = *s1++ - *s2++;
      if(d != 0) return (d);
   }

   if(*s1 == '\0' && *s2 == '\0')
      return 0;
   else
      return *s1 - *s2;
}

int k_atoi(const char * s)
{
   int res = 0;

   while(*s != '\0')
   {
      res = res * 10 + *s++ - '0';
   }

   return res;
}
