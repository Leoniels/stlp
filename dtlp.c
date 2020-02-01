/* See COPYING file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <gmp.h>

#define BASE16 16

static void
usage(void)
{
	fputs("usage: stlpd [-h] [-v] [-f crypted_key_file]\n", stderr);
	exit(EX_USAGE);
}

/* Decrypt the key resolving the time lock puzzle */
int
main (int argc, char *argv[])
{
	int i;
	FILE *fp = NULL;
	mpz_t Ck, a, t, n, key;

	/* process input */
	for (i = 1; i < argc; i++)
		if (!strcmp(argv[i], "-h"))
			usage();
		else if (!strcmp(argv[i], "-v")) {
			puts("simple time lock puzzle decrypter v0.2");
			exit(EX_OK);
		} else if (!strcmp(argv[i], "-f")) {
			fp = fopen(argv[++i], "r");
			if (!fp) {
				fprintf(stderr, "Error opening %s\n", argv[i]);
				exit(EX_NOINPUT);
			}
		}
	
	/* setup */
	mpz_init(Ck);
	mpz_init(a);
	mpz_init(t);
	mpz_init(n);
	mpz_init(key);

	/* read crypted key from stdin or file */
	mpz_inp_str(Ck, fp, 16);
	mpz_inp_str(a, fp, 16);
	mpz_inp_str(t, fp, 16);
	mpz_inp_str(n, fp, 16);
	if (fp)
		fclose(fp);

	/* resolv time lock puzzle */
	while (mpz_cmp_ui(t, 0UL)) {
		mpz_powm_ui(a, a, 2UL, n);
		mpz_sub_ui(t, t, 1UL);
	}

	/* decrypt */
	mpz_sub(key, Ck, a);
	mpz_out_str(NULL, BASE16, key); putchar('\n');

	/* unsetup */
	mpz_clear(Ck);
	mpz_clear(a);
	mpz_clear(t);
	mpz_clear(n);
	mpz_clear(key);

	return 0;
}

