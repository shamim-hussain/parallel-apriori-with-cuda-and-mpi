#include<iostream>
#include<stdio.h>
#include<unistd.h>
#include "getticks.h"

using namespace std;

int main()
{
	unsigned long long start = 0;
	unsigned long long finish = 0;
        long double result=0.0;
	unsigned int i;

	start = getticks();

	/* for(i=0; i < 1000000000; i++) */
	/* { */
	/* 	result += sqrtd((long double)i*i); */
        /* } */

	usleep(1000000); // 10 seconds sleep

	finish = getticks();

	printf("1 second usleep: finish(%llu) - start(%llu) = %llu \n", finish, start, (finish-start));
    printf("Result of function = %.10Lf seconds\n",tics_to_sec(finish-start));
	return 0;
}