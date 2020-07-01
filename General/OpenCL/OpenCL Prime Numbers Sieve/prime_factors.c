#include <stdio.h>
#include <math.h>

int main(){
   /* Declare variables */
   long number;
   int i, j, noPrime, isPrime;

   /* User input */
   printf("Enter a large number: ");
   scanf("%u", &number);

   /* Convert to positive integers */
   number = abs(number);

   /* Flush stdin */
   fseek(stdin, 0, SEEK_END);

   noPrime = 1;

   printf("Prime factors: ");

   for(i = 2; i <= number/2; i++) {

	  /* Check if number is divisible with no remainder */
      if(number % i == 0) {        
		 /* Check whether i is a prime number */
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
         if(isPrime) {
            printf("%d ", i);
		    noPrime = 0;
	     }
      }
   }

   if(noPrime)
	   printf("None, %u is a prime number.\n", number);

   /* Wait for key press before exiting */
   getchar();

   return 0;
}