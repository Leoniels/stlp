#include <stdio.h>
#include <bsd/stdlib.h>   // abs(), arc4random().
#include <gmp.h>	  // mpz stuff...
#include <sys/time.h>	  // gettimeofday()
#include <sys/resource.h> // setpriority()

#define PRIME_LENGTH 512   // Bits. Must be multiple of 4.

/*
--------------------------------------------------------------------
  Obtain a hexadecimal value where the first bit in 1 and the rest
  in 0 on a string with prefix '0X' for GMP convenience.

  The word's value will be 2 ^ (length-1).

  Still waiting to find a better definition instead this shit.
--------------------------------------------------------------------
*/
void initialValue (int length, char * hexword) {
  int i;

  hexword[length - 1] = '\0';
  hexword[0] = '0';
  hexword[1] = 'X';
  hexword[2] = '8';

  for(i = 3; i < length - 1; i++)
    hexword[i] = '0';
}


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
  Obtain the modulus n and fi(n) by creating two random primes
  values (p, q) and multiply them.

  n = p * q
  fi(n) = (p - 1) * (q - 1)
--------------------------------------------------------------------
*/
void getModulus(mpz_t n, mpz_t fiN) {
  mpz_t p, q, rand, seed;
  mpz_init(p);
  mpz_init(q);
  mpz_init(rand);
  mpz_init(seed);

  // Length of string which will represent a word of PRIME_LENGTH
  // bits in hexadecimal base.
  // (PRIME_LENGTH bits) / (4 bits every hex value)
  // plus 3 characters denoting the '0X' prefix and '\0' sufix.
  int length = PRIME_LENGTH/4 + 3;
  // Hex words strings
  char defaultWord[length];  // 2 ^ PRIME_LENGTH
  char seedStr[length];

  // Obtaining initial default value and setting it.
  initialValue(length, defaultWord);
  mpz_set_str(p, defaultWord, 0);
  mpz_set_str(q, defaultWord, 0);

  // Setting the randomizer.
  gmp_randstate_t randomizer;
  gmp_randinit_default(randomizer);

  // Obtaining random seed, converting to mpz int and setting it.
  randomSeed(length, seedStr);
  mpz_set_str(seed, seedStr, 0);
  gmp_randseed(randomizer, seed);

  // Obtaining random value and adding it to p.
  mpz_urandomb(rand, randomizer, 511);
  mpz_add(p, p, rand);
  // Same to q.
  mpz_urandomb(rand, randomizer, 511);
  mpz_add(q, q, rand);

  // Obtaining next primes of p and q
  mpz_nextprime(p, p);
  mpz_nextprime(q, q);

  // Obtaining n: n = p * q
  mpz_mul(n, p, q);

  // Obtaining p-1 and q-1
  unsigned int one = 1;
  mpz_sub_ui(p, p, one);
  mpz_sub_ui(q, q, one);

  // Obtaining fiN: fiN = (p -1) * (q - 1)
  mpz_mul(fiN, p, q);

  // Free memory
  mpz_clear(p);
  mpz_clear(q);
  mpz_clear(rand);
  mpz_clear(seed);
  gmp_randclear(randomizer);
}


/*
--------------------------------------------------------------------
  Return ratio square mod n calculated per seconds

           1million usec * squares calculated
  ratio = ------------------------------------
              seconds * 1million + usec

--------------------------------------------------------------------
*/
void getRatioSquare (double sec, double usec, mpz_t squares,
                       mpz_t actualRatio) {

  mpz_t secPlusUsec, secForSquar;
  mpz_init(secPlusUsec);
  mpz_init(secForSquar);

  mpz_mul_ui(secForSquar, squares, 1000000L);
  mpz_set_ui(secPlusUsec, (unsigned long)sec * 1000000 +
                          (unsigned long)usec);

  mpz_cdiv_q(actualRatio, secForSquar, secPlusUsec);

  // Free memory
  mpz_clear(secPlusUsec);
  mpz_clear(secForSquar);
}

/*
--------------------------------------------------------------------
  Main
--------------------------------------------------------------------
*/
int main (int argc, char *argv[]) {
  // One cicle in the main loop as default.
  int testCicles = 1;
  struct timeval t0, tf, t;
  FILE *fp;

  mpz_t n, fiN, exp, squares, base, resul, actualRatio, averRatio;
  mpz_init(n);
  mpz_init(fiN);
  mpz_init(exp);
  mpz_init(squares);
  mpz_init(base);
  mpz_init(resul);
  mpz_init(actualRatio);
  mpz_init(averRatio);


  // In case user wants to extend the test.
  if (argc >= 2)
    testCicles = atoi(argv[1]);
  // Or set the numbers of squares to calculate every test.
  // In other case default value will be 100 million.
  if (argc == 3)
    mpz_set_str(squares, argv[2], 10);
  else
    mpz_set_str(squares, "10000000", 10);

  // Load the squares it will calculate every cicle from a file.
  // Set base = 2.
  mpz_set_ui(base, 2L);

  // Process needs the real power of the CPU to get a precise value
  // of performance ratio.
  setpriority(PRIO_PROCESS, 0, -20);

  int cicles = 0;
  mpz_set_ui(averRatio, 0L);

  // Main loop. Every cicle sets a new n modulus and resolve the same
  // squares to catch the time invested in.
  while (cicles < testCicles) {
    // Obtain a random modulus n of 2*PRIME_LENGTH bits
    getModulus(n, fiN);
    mpz_set(exp, squares);
    // Resolving a square resul = base ^ (2 ^ exp) mod n.
    gettimeofday (&t0, NULL);
    mpz_set(resul, base);
    while (mpz_cmp_ui(exp, 0L) > 0) {
      mpz_powm_ui(resul, resul, 2L, n);
      mpz_sub_ui(exp, exp, 1L);
    }
    gettimeofday (&tf, NULL);

    timersub (&tf, &t0, &t);
    getRatioSquare(t.tv_sec, t.tv_usec, squares, actualRatio);
    mpz_add(averRatio, averRatio, actualRatio);
    // Traze of last results.
    printf ("Time = %ld:%ld (seg:mseg) Ratio = ", t.tv_sec,
    		t.tv_usec/1000);
    mpz_out_str(NULL, 10, actualRatio);
    printf("\n");
    cicles++;
  }

  // Final average divided by cicles
  mpz_set_ui(base, (unsigned long)testCicles);
  mpz_cdiv_q(averRatio, averRatio, base);

  printf("Average Ratio = ");
  mpz_out_str(NULL, 10, averRatio);
  printf("\n");

  // Store the results in a file.
  fp = fopen("average_square_per_seconds.txt", "w");
  mpz_out_str(fp, 10, averRatio);
  fclose(fp);

  // Free memory
  mpz_clear(n);
  mpz_clear(fiN);
  mpz_clear(exp);
  mpz_clear(squares);
  mpz_clear(base);
  mpz_clear(resul);
  mpz_clear(actualRatio);
  mpz_clear(averRatio);
}
