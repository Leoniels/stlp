#include <stdio.h>
#include <bsd/stdlib.h>   // abs(), arc4random().
#include <gmp.h>	  // mpz stuff...
#include <sys/resource.h> // setpriority()

/*
--------------------------------------------------------------------
  Return the number converted to a hexadecimal value on a char.
--------------------------------------------------------------------
*/
char getHex(int num) {
  num = abs(num % 16);

  switch(num)
  {
    case 10 :
    	return 'A';
    case 11 :
    	return 'B';
    case 12 :
    	return 'C';
    case 13 :
    	return 'D';
    case 14 :
    	return 'E';
    case 15 :
    	return 'F';
    default :
    	return (char) num + 48;   // ASCI numbers begin at index 48
  }
}

/*
--------------------------------------------------------------------
  Calls arc4random to get random hexadecimal numbers and fill the
  array with a random hexadecimal word on a string type.
--------------------------------------------------------------------
*/
void randomSeed(int length, char * randomData) {
  int i;

  for(i = 2; i < length - 1; i++)
    randomData[i] = getHex(arc4random());

  randomData[0] = '0';
  randomData[1] = 'X';
  randomData[length - 1] = '\0';
}

/*
--------------------------------------------------------------------
  Main
--------------------------------------------------------------------
*/
int main (int argc, char *argv[]) {
  int length;
  
  if (argc != 2) {
    printf ("Introduce key length in bits\n");
    return 0;
  }

  // We add 3 chars.
  length = atoi(argv[1]);

  char seedStr[length+3];
  mpz_t seed, key;
  mpz_init(seed);
  mpz_init(key);

  // Setting the randomizer.
  gmp_randstate_t randomizer;
  gmp_randinit_default(randomizer);

  // Obtaining random seed, converting to mpz int and setting it.
  randomSeed(length+3, seedStr);
  mpz_set_str(seed, seedStr, 0);
  gmp_randseed(randomizer, seed);

  // Random key with length in bits
  mpz_urandomb(key, randomizer, length);

  // Store data
  FILE *fp;
  fp = fopen("key.txt", "w");
  mpz_out_str(fp, 10, key);
  fclose(fp);

  // Free memory
  mpz_clear(key);
  mpz_clear(seed);
}