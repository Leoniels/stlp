#include <stdio.h>	  // Do I need to tell u why?
#include <gmp.h>	  // mpz stuff...
#include <sys/time.h>     // gettimeofday()
#include <sys/resource.h> // setpriority()

/*
--------------------------------------------------------------------
  Main
--------------------------------------------------------------------
*/
int main (int argc, char *argv[]) {
  struct timeval t0, tf, t;
  FILE *fp;

  mpz_t n, base, powMod, time, key, encryptedKey, exp;
  mpz_init(n);
  mpz_init(base);
  mpz_init(powMod);
  mpz_init(time);
  mpz_init(key);
  mpz_init(encryptedKey);
  mpz_init(exp);

  // Loading data from file
  fp = fopen("key_encrypted.txt", "r");
  mpz_inp_str(encryptedKey, fp, 16);
  mpz_inp_str(n, fp, 16);
  mpz_inp_str(base, fp, 16);
  mpz_inp_str(time, fp, 16);
  fclose(fp);
  mpz_out_str(NULL, 16, encryptedKey);
  printf("\n\n");
  mpz_out_str(NULL, 16, n);
  printf("\n\n");
  mpz_out_str(NULL, 16, base);
  printf("\n\n");
  mpz_out_str(NULL, 16, time);
  printf("\n\n");

  // We need high priority to get the key as soos as it can.
  setpriority(PRIO_PROCESS, 0, -20);

  // Calculate pow in modulus
  gettimeofday (&t0, NULL);

  // powMod = base ^ (2 ^ exp) mod n.
  mpz_set(powMod, base);

  while (mpz_cmp_ui(time, 0L) > 0) {
    mpz_powm_ui(powMod, powMod, 2L, n);
    mpz_sub_ui(time, time, 1L);
  }
  gettimeofday (&tf, NULL);

  // Calculate subtraction and get key
  mpz_sub(key, encryptedKey, powMod);

  // Show the key decrypted
  printf("Key decripted: \n");
  mpz_out_str(NULL, 10, key);
  printf("\n\n");

  // Show time results
  timersub (&tf, &t0, &t);
  printf ("Time = %ld:%ld (seg:mseg).\n", t.tv_sec, t.tv_usec/1000);

  // Free memory
  mpz_clear(n);
  mpz_clear(base);
  mpz_clear(powMod);
  mpz_clear(time);
  mpz_clear(key);
  mpz_clear(encryptedKey);
}
