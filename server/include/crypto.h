#ifndef CRYPTO_H
#define CRYPTO_H

#define CRYPTO_SALT_LEN 32
#define CRYPTO_HASH_LEN 65

/* Fill out with a random hex salt (CRYPTO_SALT_LEN + 1 bytes) */
int crypto_gen_salt(char *salt_out);

/* Hash password with salt into hash_out (CRYPTO_HASH_LEN bytes) */
int crypto_hash_password(const char *password, const char *salt, char *hash_out);

/* Returns 1 if password matches stored hash+salt, 0 otherwise */
int crypto_verify_password(const char *password, const char *salt, const char *stored_hash);

#endif /* CRYPTO_H */