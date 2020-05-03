#ifndef __GETTICKS_H
#define __GETTICKS_H


#include<stdio.h>

/****************************************************************/
// FOR POWER9 SYSTEMS ONLY - x86 SYSTEMS HAVE A DIFFERENT CODE  //
/****************************************************************/

typedef unsigned long long ticks;

#define _TICK_PER_SECOND 512000000.0L

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

long double inline ticks_to_sec(ticks tick_count){
  return ((long double)tick_count)/_TICK_PER_SECOND;
}

// int main()
// {
// 	unsigned long long start = 0;
// 	unsigned long long finish = 0;
//         long double result=0.0;
// 	unsigned int i;

// 	start = getticks();

// 	/* for(i=0; i < 1000000000; i++) */
// 	/* { */
// 	/* 	result += sqrtd((long double)i*i); */
//         /* } */

// 	usleep(10000000); // 10 seconds sleep

// 	finish = getticks();

// 	printf("10 second usleep: finish(%llu) - start(%llu) = %llu \n", finish, start, (finish-start));

// 	return 0;
// }
#endif