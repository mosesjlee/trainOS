
#include <kernel.h>


int k_strlen(const char* str)
{
   int length = 0;

   while(*str++ != NULL) length++;

   return length;

}

void* k_memcpy(void* dst, const void* src, int len)
{
   char * p1 = (char *) dst;
   char * p2 = (char *) src;
   while(len-- != 0){
      *p1++ = *p2++;
   }

   return dst;
}

int k_memcmp(const void* b1, const void* b2, int len)
{

   char * p1 = (char *) b1;
   char * p2 = (char *) b2;
   while(len-- != 0){
      if(*p1 != *p2) return *p1 - *p2;
      ++p1;
      ++p2;
   }

   return 0;
}

