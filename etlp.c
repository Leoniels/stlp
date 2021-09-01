/* See COPYING file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sysexits.h>
#include <gmp.h>

#define DEF_TESTIME 1 /* seconds */
#define SQUARES_PER_CYCLE 1000UL /* squares to do each cycle */
#define BASE16 16
#define BASE10 10

static FILE *key_file;
static unsigned long int time_enc, test_time = DEF_TESTIME;
static unsigned long int bit_length = 1024; /* modulus length */
static unsigned long int S;
static gmp_randstate_t random_gen;
static mpz_t n, phi_n;
static mpz_t Ck, b, a, e, t;

static void
usage(void)
{
	fputs("usage: etlp [-h] [-v] [-t test_time] [-f key_file] [-b bits] time_encrypted\n", stderr);
	exit(EX_USAGE);
}

static void
setup(void)
{
	/* Initialize mpz_t global variables */
	mpz_init(n);
	mpz_init(phi_n);
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

/* Obtain the modulus n and phi(n) by creating two large random primes
 * (p, q) and multiplying them.
 *
 * n = p * q
 * phi(n) = (p - 1) * (q - 1) */
static void
gen_modulus(void)
{
	mpz_t p, q;
	mpz_init(p);
	mpz_init(q);

	mpz_urandomb(p, random_gen, bit_length/2);
	mpz_urandomb(q, random_gen, bit_length/2);

	mpz_nextprime(p, p);
	mpz_nextprime(q, q);

	mpz_mul(n, p, q);

	mpz_sub_ui(p, p, 1UL);
	mpz_sub_ui(q, q, 1UL);

	mpz_mul(phi_n, p, q);

	mpz_clear(p);
	mpz_clear(q);
}

/* random base given this condition 1 < a < n */
static void
gen_base(void)
{
	mpz_urandomb(a, random_gen, bit_length);

	mpz_sub_ui(n, n, 2UL);
	mpz_mod(a, a, n);
	mpz_add_ui(n, n, 2UL);
	mpz_add_ui(a, a, 2UL);
}

/* test performance to obtain squaring modulo n per second (S) */
static void
test_perf(void)
{
	unsigned long cycles = 0;
	clock_t t0, ticks, t_test;

	mpz_set(b, a);
	ticks = CLOCKS_PER_SEC * test_time;
	t0 = clock();
	t_test = t0 + ticks;
	do {
		mpz_set_ui(t, SQUARES_PER_CYCLE);
		while(mpz_cmp_ui(t, 0) > 0) {
			mpz_powm_ui(b, b, 2UL, n);
			mpz_sub_ui(t, t, 1UL);
		}
		cycles++;
	} while(clock() < t_test);
	S = (unsigned long)((SQUARES_PER_CYCLE * cycles)/test_time);
}

/* Encrypt efficiently by solving: Ck = k + b
 * b = a ^ e mod n
 * e = 2 ^ t mod phi_n */
static void
encrypt(void)
{
	mpz_t T, two, key;
	mpz_init(T);
	mpz_init(two);
	mpz_init(key);

	/* calculate challenge to reach desired time */
	mpz_set_ui(T, time_enc);
	mpz_mul_ui(t, T, S);

	/* calculate b */
	mpz_set_ui(two, 2UL);
	mpz_powm(e, two, t, phi_n);
	mpz_powm(b, a, e, n);

	/* read key from stdin */
	if (mpz_inp_str(key, key_file, BASE16) == 0) {
		fputs("Error reading key from stdin", stderr);
		exit(EX_IOERR);
	}

	if (mpz_cmp(key, n) >= 0) {
		fputs("Key is too large for modulus length", stderr);
		exit(EX_USAGE);
	}

	/* encrypt key with b */
	mpz_add(Ck, b, key);
	
	mpz_clear(T);
	mpz_clear(two);
}

static void
unsetup(void)
{
	if(key_file != stdin)
		fclose(key_file);
	mpz_clear(n);
	mpz_clear(phi_n);
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
	key_file = stdin;

	time_enc = 0;
	/* process input */
	for (i = 1; i < argc; i++)
		if (!strcmp(argv[i], "-h"))
			usage();
		else if (!strcmp(argv[i], "-v")) {
			puts("simple time lock puzzle encrypter v0.3");
			exit(EX_OK);
		} else if (!strcmp(argv[i], "-t"))
			test_time = strtoul(argv[++i], NULL, BASE10);
		else if (!strcmp(argv[i], "-b")) {
			bit_length = strtoul(argv[++i], NULL, BASE10);
			if (bit_length % 2) {
				fputs("Bit length should be a multiple of 2", stderr);
				exit(EX_USAGE);
			}
		} else if (!strcmp(argv[i], "-f")) {
			key_file = fopen(argv[++i], "r");
			if (!key_file) {
				fprintf(stderr, "Error opening %s\n", argv[i]);
				exit(EX_NOINPUT);
			}
		} else if (time_enc == 0)
			time_enc = strtoul(argv[i], NULL, BASE10);
		else
			usage();
	
	if (time_enc == 0) usage();

	setup();

	gen_modulus();
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

