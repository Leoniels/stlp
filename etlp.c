#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sysexits.h>
#include <gmp.h>

#define DEF_TESTIME 1 /* second */
#define PRIME_LEN 512 /* prime bit length */
#define BASE16 16
#define BASE10 10

static unsigned long int time_enc, test_time = DEF_TESTIME;
static unsigned long int S;
static gmp_randstate_t random_gen;
static mpz_t n, fi_n;
static mpz_t Ck, b, a, e, t;

/*#include "config.h"*/

static void
usage(void)
{
	fputs("usage: stlpe [-h] [-v] [-t test_time] time_encrypted\n", stderr);
	exit(EX_USAGE);
}

static void
setup(void)
{
	/* Initialize mpz_t global variables */
	mpz_init(n);
	mpz_init(fi_n);
	mpz_init(Ck);
	mpz_init(a);
	mpz_init(b);
	mpz_init(e);
	mpz_init(t);

	/* Setup gmp random generator */
	FILE *fp;
	unsigned long int random_seed;

	fp = fopen("/dev/urandom", "r");
	fscanf(fp, "%lu", &random_seed);
	fclose(fp);

	gmp_randinit_default(random_gen);
	gmp_randseed_ui(random_gen, random_seed);
}

/* Obtain the modulus n and fi(n) by creating two big random primes values
 * (p, q) and multiply them.
 *
 * n = p * q
 * fi(n) = (p - 1) * (q - 1) */
static void
gen_modulos(void)
{
	mpz_t p, q;
	mpz_init(p);
	mpz_init(q);

	mpz_urandomb(p, random_gen, PRIME_LEN);
	mpz_urandomb(q, random_gen, PRIME_LEN);

	mpz_nextprime(p, p);
	mpz_nextprime(q, q);

	mpz_mul(n, p, q);

	mpz_sub_ui(p, p, 1UL);
	mpz_sub_ui(q, q, 1UL);

	mpz_mul(fi_n, p, q);

	mpz_clear(p);
	mpz_clear(q);
}

/* random base given this condition 1 < a < n */
static void
gen_base(void)
{
	mpz_urandomb(a, random_gen, PRIME_LEN*2);

	mpz_sub_ui(n, n, 2UL);
	mpz_mod(a, a, n);
	mpz_add_ui(n, n, 2UL);
	mpz_add_ui(a, a, 2UL);
}

/* test performance to obtain squaring modulo n per second (S) */
static void
test_perf(void)
{
	mpz_set_ui(t, ULONG_MAX);
	mpz_set(b, a);
	clock_t t0 = clock();
	clock_t t1 = 0;
	while(t1/CLOCKS_PER_SEC < test_time) {
		mpz_powm_ui(b, b, 2UL, n);
		mpz_sub_ui(t, t, 1UL);
		t1 = clock() - t0;
	}
	unsigned long squares = ULONG_MAX - mpz_get_ui(t);
	S = (unsigned long int)(squares/test_time);
}

/* Encrypt efficiently by solving: Ck = k + b
 * b = a ^ e mod n
 * e = 2 ^ t mod fi_n */
static void
encrypt(void)
{
	unsigned int key;
	mpz_t T, two;
	mpz_init(T);
	mpz_init(two);

	/* calculate challenge to reach desired time */
	mpz_set_ui(T, time_enc);
	mpz_mul_ui(t, T, S);

	/* calculate b */
	mpz_set_ui(two, 2UL);
	mpz_powm(e, two, t, fi_n);
	mpz_powm(b, a, e, n);

	/* read key from stdin */
	if (scanf("%x", &key) == EOF) {
		fputs("Error reading key from stdin", stderr);
		exit(EX_IOERR);
	}

	/* encrypt key with b */
	mpz_add_ui(Ck, b, key);
	
	mpz_clear(T);
	mpz_clear(two);
}

static void
unsetup(void)
{
	mpz_clear(n);
	mpz_clear(fi_n);
	mpz_clear(Ck);
	mpz_clear(t);
	mpz_clear(a);
	mpz_clear(e);
	gmp_randclear(random_gen);
}

/* Encrypt the hexadecimal key received from stdin for the seconds specified
 * in the argument. */
int
main (int argc, char *argv[])
{
	int i;

	/* process input */
	for (i = 1; i < argc; i++)
		if (!strcmp(argv[i], "-h"))
			usage();
		else if (!strcmp(argv[i], "-v")) {
			puts("simple time lock puzzle encrypter v0.2");
			exit(EX_OK);
		} else if (!strcmp(argv[i], "-t"))
			test_time = strtoul(argv[++i], NULL, BASE10);
	time_enc = strtoul(argv[argc-1], NULL, BASE10);

	setup();

	gen_modulos();
	gen_base();
	test_perf();

	encrypt();
	
	/* output Ck, a, t, n to stdout */
	mpz_out_str(NULL, BASE16, Ck); putchar('\n');
	mpz_out_str(NULL, BASE16, a);  putchar('\n');
	mpz_out_str(NULL, BASE16, t);  putchar('\n');
	mpz_out_str(NULL, BASE16, n);  putchar('\n');

	unsetup();

	return 0;
}

