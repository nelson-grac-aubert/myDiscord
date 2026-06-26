#ifndef MESSAGE_CRYPTO_H
#define MESSAGE_CRYPTO_H

#include "../network/include/packet.h"

/*
 * Chiffre plaintext → cipher_b64 + iv_b64
 * À appeler juste avant packet_build(MSG_SEND, ...)
 */
int message_crypto_encrypt_field(const char *plaintext,
                                  char       *cipher_b64,  /* out ≥ 512 */
                                  char       *iv_b64);     /* out ≥ 32  */

/*
 * Déchiffre cipher_b64 + iv_b64 → plaintext_out
 * À appeler à la réception d'un SERVER_PUSH
 */
int message_crypto_decrypt_field(const char *cipher_b64,
                                  const char *iv_b64,
                                  char       *plaintext_out,
                                  int         out_size);

#endif