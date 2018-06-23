#include <stdio.h>
#include <gmp.h>	  // mpz stuff...
#include <sys/time.h>     // gettimeofday()
#include <sys/resource.h> // setpriority()

/*
--------------------------------------------------------------------
  Decrypt the key from the file with the ratio data stored.
--------------------------------------------------------------------
*/
int main (int argc, char *argv[]) {
  struct timeval time0, timef, timet;
  FILE *fp;

  mpz_t key, encryptedKey, powMod, base, t, n;
  mpz_init(key);
  mpz_init(encryptedKey);
  mpz_init(powMod);
  mpz_init(base);
  mpz_init(t);
  mpz_init(n);

  // Loading data from file
  fp = fopen("key_encrypted.txt", "r");
  if (!fp) {
    printf("key_encrypted.txt not found");
    return 1;
  }
  mpz_inp_str(encryptedKey, fp, 16);
  mpz_inp_str(n, fp, 16);
  mpz_inp_str(base, fp, 16);
  mpz_inp_str(t, fp, 16);
  fclose(fp);
  mpz_out_str(NULL, 16, encryptedKey);
  printf("\n\n");
  mpz_out_str(NULL, 16, n);
  printf("\n\n");
  mpz_out_str(NULL, 16, base);
  printf("\n\n");
  mpz_out_str(NULL, 16, t);
  printf("\n\n");

  // We need high priority to get the key as soos as it can.
  setpriority(PRIO_PROCESS, 0, -20);

  // Calculate pow in modulus
  gettimeofday (&time0, NULL);

  // powMod = base
  mpz_set(powMod, base);

  while (mpz_cmp_ui(t, 0L) > 0) {
    mpz_powm_ui(powMod, powMod, 2L, n);
    mpz_sub_ui(t, t, 1L);
  }
  gettimeofday (&timef, NULL);

  // Calculate subtraction and get key
  mpz_sub(key, encryptedKey, powMod);

  // Show the key decrypted
  printf("Key decripted: \n");
  mpz_out_str(NULL, 10, key);
  printf("\n\n");

  // Show time results
  timersub (&timef, &time0, &timet);
  printf ("Time = %ld:%ld (seg:mseg).\n", timet.tv_sec, timet.tv_usec/1000);

  fp = fopen("key_decrypted.txt", "w");
  mpz_out_str(fp, 10, key);
  fputc('\n', fp);
  fprintf(fp, "%ld:%ld\n", timet.tv_sec, timet.tv_usec/1000);
  fclose(fp);

  // Free memory
  mpz_clear(key);
  mpz_clear(encryptedKey);
  mpz_clear(powMod);
  mpz_clear(base);
  mpz_clear(t);
  mpz_clear(n);

  return 0;
}
