
#ifndef MESSAGE_CRYPTO_H
#define MESSAGE_CRYPTO_H

#include <libpq-fe.h>

/* Chiffre plaintext → iv_b64 + cipher_b64 */
int message_crypto_encrypt(const char *plaintext,
                            char       *iv_b64,
                            char       *cipher_b64);

/* Déchiffre cipher_b64 + iv_b64 → plaintext_out */
int message_crypto_decrypt(const char *cipher_b64,
                            const char *iv_b64,
                            char       *plaintext_out,
                            int         out_size);

/* Chiffre ET insère en base — à appeler depuis message.c */
int message_crypto_encrypt_and_insert(PGconn     *conn,
                                       const char *plaintext,
                                       int         id_author,
                                       int         id_channel);

#endif /* MESSAGE_CRYPTO_H */