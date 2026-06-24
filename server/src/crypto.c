#include "../include/crypto.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <string.h>

int crypto_gen_salt(char *salt_out)
{
    unsigned char raw[CRYPTO_SALT_LEN / 2];

    if (RAND_bytes(raw, sizeof(raw)) != 1)
        return -1;

    for (int i = 0; i < (int)sizeof(raw); i++)
        sprintf(salt_out + (i * 2), "%02x", raw[i]);

    salt_out[CRYPTO_SALT_LEN] = '\0';
    return 0;
}

int crypto_hash_password(const char *password, const char *salt, char *hash_out)
{
    char salted[512];
    snprintf(salted, sizeof(salted), "%s%s", salt, password);

    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char *)salted, strlen(salted), digest);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(hash_out + (i * 2), "%02x", digest[i]);

    hash_out[CRYPTO_HASH_LEN - 1] = '\0';
    return 0;
}

int crypto_verify_password(const char *password, const char *salt, const char *stored_hash)
{
    char computed[CRYPTO_HASH_LEN];

    if (crypto_hash_password(password, salt, computed) != 0)
        return 0;

    return memcmp(computed, stored_hash, CRYPTO_HASH_LEN - 1) == 0 ? 1 : 0;
}