#include <utils.h>
#include <types.h>

#include <mm_address.h>
#include <mm.h>

extern struct Shared_frame shared_frames[10]; 

void copy_data(void *start, void *dest, int size)
{
  DWord *p = start, *q = dest;
  Byte *p1, *q1;
  while(size > 4) {
    *q++ = *p++;
    size -= 4;
  }
  p1=(Byte*)p;
  q1=(Byte*)q;
  while(size > 0) {
    *q1++ = *p1++;
    size --;
  }
}
/* Copia de espacio de usuario a espacio de kernel, devuelve 0 si ok y -1 si error*/
int copy_from_user(void *start, void *dest, int size)
{
  DWord *p = start, *q = dest;
  Byte *p1, *q1;
  while(size > 4) {
    *q++ = *p++;
    size -= 4;
  }
  p1=(Byte*)p;
  q1=(Byte*)q;
  while(size > 0) {
    *q1++ = *p1++;
    size --;
  }
  return 0;
}
/* Copia de espacio de kernel a espacio de usuario, devuelve 0 si ok y -1 si error*/
int copy_to_user(void *start, void *dest, int size)
{
  DWord *p = start, *q = dest;
  Byte *p1, *q1;
  while(size > 4) {
    *q++ = *p++;
    size -= 4;
  }
  p1=(Byte*)p;
  q1=(Byte*)q;
  while(size > 0) {
    *q1++ = *p1++;
    size --;
  }
  return 0;
}

/* access_ok: Checks if a user space pointer is valid
 * @type:  Type of access: %VERIFY_READ or %VERIFY_WRITE. Note that
 *         %VERIFY_WRITE is a superset of %VERIFY_READ: if it is safe
 *         to write to a block, it is always safe to read from it
 * @addr:  User space pointer to start of block to check
 * @size:  Size of block to check
 * Returns true (nonzero) if the memory block may be valid,
 *         false (zero) if it is definitely invalid
 */
int access_ok(int type, const void * addr, unsigned long size)
{
  unsigned long addr_ini, addr_fin;
  page_table_entry *PT = get_PT(current());

  addr_ini=(((unsigned long)addr)>>12);
  addr_fin=((((unsigned long)addr)+size)>>12);
  if (addr_fin < addr_ini) return 0; //This looks like an overflow ... deny access

  int error = 0;
  int ret = 0;  
  unsigned long i = 0; 

  switch(type)
  { 
    case VERIFY_WRITE:
      /* Should suppose no support for automodifyable code */
      if ((addr_ini>=USER_FIRST_PAGE+NUM_PAG_CODE) && (addr_fin<=USER_FIRST_PAGE+TOTAL_PAGES)) {
        for (i = addr_ini; i < addr_fin; ++i) {
          if (!PT[i].bits.present) {
            error = 1; 
            break;
          } 
        }
        if (!error) ret = 1;
      } 
      break; 

    default:
      if ((addr_ini>=USER_FIRST_PAGE+NUM_PAG_CODE) && (addr_fin<=USER_FIRST_PAGE+TOTAL_PAGES)) {
        for (i = addr_ini; i < addr_fin; ++i) {
          if (!PT[i].bits.present) {
            error = 1; 
            break;
          } 
        }
        if (!error) ret = 1;
      } 
  }
  return ret;
}


#define CYCLESPERTICK 109000

/*
 * do_div() is NOT a C function. It wants to return
 * two values (the quotient and the remainder), but
 * since that doesn't work very well in C, what it
 * does is:
 *
 * - modifies the 64-bit dividend _in_place_
 * - returns the 32-bit remainder
 *
 * This ends up being the most efficient "calling
 * convention" on x86.
 */
#define do_div(n,base) ({ \
        unsigned long __upper, __low, __high, __mod, __base; \
        __base = (base); \
        asm("":"=a" (__low), "=d" (__high):"A" (n)); \
        __upper = __high; \
        if (__high) { \
                __upper = __high % (__base); \
                __high = __high / (__base); \
        } \
        asm("divl %2":"=a" (__low), "=d" (__mod):"rm" (__base), "0" (__low), "1" (__upper)); \
        asm("":"=A" (n):"a" (__low),"d" (__high)); \
        __mod; \
})


#define rdtsc(low,high) \
        __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

unsigned long get_ticks(void) {
        unsigned long eax;
        unsigned long edx;
        unsigned long long ticks;

        rdtsc(eax,edx);

        ticks=((unsigned long long) edx << 32) + eax;
        do_div(ticks,CYCLESPERTICK);

        return ticks;
}

void memset(void *s, unsigned char c, int size)
{
  unsigned char *m=(unsigned char *)s;
  
  int i;
  
  for (i=0; i<size; i++)
  {
    m[i]=c;
  }
}
