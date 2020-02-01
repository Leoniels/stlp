# PRANVI
A simple time lock puzzle implementation.

Simple UNIX-like/suckless implementation of the cryptosystem defined in: "Time-lock puzzles and timed-release Crypto" by Ronald R. Rivest, Adi Shamir and David A. Wagner.

## Dependencies
· GNU MP (libgmp)

## Build
A simple call to make should do it fine in a Linux system at the moment.

## Example usage
Using stdin to encrypt a hexadecimal key for ten seconds and then decrypt it to stdout.
```bash
 $ echo 89ABC | etlp 10 | dtlp
```
## TODO things

· Keyfile/keyvalue as input argument in etlp.

· Manual.

· Install and uninstall make.

· Store every X time the non-resolved puzzle in a cache file.

· Make distribution package.
