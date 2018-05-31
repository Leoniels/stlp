#include <stdio.h>
#include <sys/time.h>	  // gettimeofday()
#include <sys/resource.h> // setpriority()

#include "math.h"

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
  if (argc >= 2) testCicles = atoi(argv[1]);
  // Or set the numbers of squares to calculate every test.
  // In other case default value will be 100 million.
  else if (argc == 3) mpz_set_str(squares, argv[2], 10);
  else mpz_set_str(squares, "10000000", 10);

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
  if (!fp) {
    printf("average_square_per_seconds.txt not found");
    return 1;
  } 
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
  return 0;
}
