#include <stdio.h>	  // Do I need to tell u why?
#include <bsd/stdlib.h>   // abs(), arc4random().
#include <gmp.h>	  // mpz stuff...
#include <sys/resource.h> // setpriority()

#define PRIME_LENGTH 512   // Bits. Must be multiple of 4.

/*
--------------------------------------------------------------------
  Obtain a hexadecimal value where the first bit in 1 and the rest
  in 0 on a string with prefix '0X' for GMP convenience.

  The word's value will be 2 ^ (length-1).

  I'm still watching a for a better explanation becoz this is shit.
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
  Sets base as a random value from 2 to n.
--------------------------------------------------------------------
*/
void getRandomBase(mpz_t base, mpz_t n) {
  int length = PRIME_LENGTH/4 + 3;
  char seedStr[length];
  mpz_t seed;
  mpz_init(seed);

  // Setting the randomizer.
  gmp_randstate_t randomizer;
  gmp_randinit_default(randomizer);

  // Obtaining random seed, converting to mpz int and setting it.
  randomSeed(length, seedStr);
  mpz_set_str(seed, seedStr, 0);
  gmp_randseed(randomizer, seed);

  // Random 1024 bit length base.
  mpz_urandomb(base, randomizer, 1024);

  // To control the range we will reduce in two the modulus n
  unsigned int two = 2;
  mpz_sub_ui(n, n, two);
  // Obtain the modulus
  mpz_mod(base, base, n);
  // Return n to its normal value
  mpz_add_ui(n, n, two);
  // Add two to base so it would never be 1 or 0.
  mpz_add_ui(base, base, two);

  // Free memory
  mpz_clear(seed);
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
  Main
--------------------------------------------------------------------
*/
int main (int argc, char *argv[]) {
  long int secondsEncrypted;
  FILE *fp;

  mpz_t n, fiN, exp, two, base, powMod, time, key, encryptedKey;
  mpz_init(n);
  mpz_init(fiN);
  mpz_init(exp);
  mpz_init(two);
  mpz_init(base);
  mpz_init(powMod);
  mpz_init(time);
  mpz_init(key);
  mpz_init(encryptedKey);

  // Capture from argument the time the key will need to be
  // decrypted.
  if (argc != 2) {
    printf ("Introduce encrypt time (seconds)\n");
    return 0;
  }
  secondsEncrypted = atol(argv[1]);

  // Read the ratio square per seconds stored in file.
  fp = fopen("average_square_per_seconds.txt", "r");
  mpz_inp_str(time, fp, 10);
  fclose(fp);

  // Calculate T parameter (T = time(seconds) * ratio(square per
  // per seconds).
  mpz_mul_si(time, time, secondsEncrypted);

  // Obtain modulus
  getModulus(n, fiN);

  // Obtaining random base.
  getRandomBase(base, n);

  // exp = 2 ^ time mod fi(n)
  mpz_set_ui (two, 2L);
  mpz_powm(exp, two, time, fiN);

  // powMod = 2 ^ exp mod n
  mpz_powm(powMod, base, exp, n);

  // Get key value and load it into a mpz_int
  fp = fopen("key.txt", "r");
  mpz_inp_str(key, fp, 10);
  fclose(fp);
  // Obtain encrypted key
  mpz_add(encryptedKey, key, powMod);

  // Store data to next decrypt
  fp = fopen("key_encrypted.txt", "w");
  mpz_out_str(fp, 16, encryptedKey);
  fputc('\n', fp);
  mpz_out_str(fp, 16, n);
  fputc('\n', fp);
  mpz_out_str(fp, 16, base);
  fputc('\n', fp);
  mpz_out_str(fp, 16, time);
  fclose(fp);

  // Free memory
  mpz_clear(n);
  mpz_clear(fiN);
  mpz_clear(exp);
  mpz_clear(two);
  mpz_clear(base);
  mpz_clear(powMod);
  mpz_clear(time);
  mpz_clear(key);
  mpz_clear(encryptedKey);
}
