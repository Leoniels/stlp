# Programa-de-antivicio
A time lock puzzle implementation. 

This early prototipe is a test of a time lock puzzle based on the paper "Time-lock puzzles and timed-release Crypto" by Ronald R. Rivest, Adi Shamir and David A. Wagner.

To compile the project you must have the GNU MP ver. 6.1.2 library and libbsd-dev installed and type for every source file:

  gcc source.c -lgmp -lbsd

Yeah I know, I should write a Makefile to avoid this shit.

To run this you must launch all the binarys from the same directory so every process can reach the .txt files generated (Ok, ok... this is also a crap). Then do the following steps:

1.- First get the performance of your CPU under this task to obtain the average of squares modulus n per seconds.
    Just run square_tester binary. Optionally you can introduce the number of test to take as first argument and the number of squares modulus n to calculate every test as second argument.
    Actual default values are 1 test and 10 million squares modulus n every test. 

2.- Write in a file named 'key.txt' the key you want to encrypt in base 10, no matter length.

3.- Run the encrypt_key binary introducing the number of seconds you want to invest in later decrypt time.
    A file 'key_encrypted.txt' will be generated with the crypted key and all the data needed to decrypt it by resolving the time lock puzzle.
    It is important to run this process with root privileges so it can increase his priority.

Finaly to decrypt the key just run the decrypt_key binary and wait until the puzzle is resolved.
Same as above run this part as root to give the higher priority to the process.
