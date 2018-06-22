#include <stdio.h>
#include <sys/resource.h> // setpriority()

#include "math.h"

/*
--------------------------------------------------------------------
  Crypt the key from the file for the time specified by argument.
--------------------------------------------------------------------
*/
int main (int argc, char *argv[]) {
  long int secondsEncrypted;
  FILE *fp;

  mpz_t encryptedKey, key, powMod, base, e, two, t, n, fiN;
  mpz_init(encryptedKey);
  mpz_init(key);
  mpz_init(powMod);
  mpz_init(base);
  mpz_init(e);
  mpz_init(two);
  mpz_init(t);
  mpz_init(n);
  mpz_init(fiN);

  // Capture from argument the time the key will need to be
  // decrypted.
  if (argc != 2) {
    printf ("Introduce encrypt t (seconds)\n");
    return 1;
  }
  secondsEncrypted = atol(argv[1]);

  // Read the ratio square per seconds stored in file.
  fp = fopen("average_square_per_seconds.txt", "r");
  if (!fp) {
    printf("average_square_per_seconds.txt not found");
    return 1;
  }
  mpz_inp_str(n, fp, 16);
  mpz_inp_str(fiN, fp, 16);
  mpz_inp_str(t, fp, 10);
  fclose(fp);

  // Calculate T parameter (T = t(seconds) * ratio(square per
  // per seconds).
  mpz_mul_si(t, t, secondsEncrypted);

  // Obtaining random base.
  getRandomBase(base, n);

  // e = 2 ^ t mod fi(n)
  mpz_set_ui (two, 2L);
  mpz_powm(e, two, t, fiN);

  // powMod = a ^ e mod n
  mpz_powm(powMod, base, e, n);

  // Get key value and load it into a mpz_int
  fp = fopen("key.txt", "r");
  if (!fp) {
    printf("key.txt not found");
    return 1;
  }
  mpz_inp_str(key, fp, 10);
  fclose(fp);

  // Obtain encrypted key
  mpz_add(encryptedKey, key, powMod);

  // Store data to next decrypt
  fp = fopen("key_encrypted.txt", "w");
  if (!fp) {
    printf("key_encrypted.txt not found");
    return 1;
  }
  mpz_out_str(fp, 16, encryptedKey);
  fputc('\n', fp);
  mpz_out_str(fp, 16, n);
  fputc('\n', fp);
  mpz_out_str(fp, 16, base);
  fputc('\n', fp);
  mpz_out_str(fp, 16, t);
  fclose(fp);

  // Free memory
  mpz_clear(encryptedKey);
  mpz_clear(key);
  mpz_clear(powMod);
  mpz_clear(base);
  mpz_clear(e);
  mpz_clear(two);
  mpz_clear(t);
  mpz_clear(n);
  mpz_clear(fiN);

  return 0;
}
