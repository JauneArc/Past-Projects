#include <stdlib.h>
#include <stdio.h>
#include <math.h>
  
int main(){
   /* Declare variables */
   int start, end, i, j, isPrime;

   /* User input */
   printf("Enter start of range: ");
   scanf("%d", &start);

   printf("Enter end of range: ");
   scanf("%d", &end);

   /* Flush stdin */
   fseek(stdin, 0, SEEK_END);

   /* Check input */
   /* Convert to positive integers */
   start = abs(start);
   end = abs(end);

   if(start > end || start < 1) {
      printf("Invalid range.\n");
	  exit(1);
   }

   /* Find prime numbers within the given range */
   printf("Prime numbers between %d and %d:\n", start, end);

   /* 1 is not considered to be a prime number */
   if(start == 1)
	   start = 2;

   for(i = start; i <= end; i++) {
	  /* Set prime number flag */
      isPrime = 1; 

	  /* Check whether i is prime */
      for(j = 2; j <= i/2; j++) {
	     /* if a division has no remainder, i cannot be prime */
         if(i % j == 0) {
            isPrime = 0;
            break;
         }
      }
       
	  /* Output prime number */
      if(isPrime)
         printf("%d ", i);
   }

   /* Wait for key press before exiting */
   getchar();

   return 0;
}