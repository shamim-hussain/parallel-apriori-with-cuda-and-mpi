#ifndef __GETTICKS_H
#define __GETTICKS_H


#include<stdio.h>

/****************************************************************/
// FOR POWER9 SYSTEMS ONLY - x86 SYSTEMS HAVE A DIFFERENT CODE  //
/****************************************************************/

// dtype of ticks
typedef unsigned long long ticks;

// DCS POWER9 TICKS PER SECOND  
#define _TICK_PER_SECOND 512000000.0L

// Get ticks from the cpu clock
static __inline__ ticks getticks(void)
{
  unsigned int tbl, tbu0, tbu1;

  do {
    __asm__ __volatile__ ("mftbu %0" : "=r"(tbu0));
    __asm__ __volatile__ ("mftb %0" : "=r"(tbl));
    __asm__ __volatile__ ("mftbu %0" : "=r"(tbu1));
  } while (tbu0 != tbu1);

  return (((unsigned long long)tbu0) << 32) | tbl;
}


// Convert ticks to seconds
long double inline ticks_to_sec(ticks tick_count){
  return ((long double)tick_count)/_TICK_PER_SECOND;
}
#endif