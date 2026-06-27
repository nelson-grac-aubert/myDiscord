#ifndef MESSAGE_CRYPTO_H
#define MESSAGE_CRYPTO_H

#include "packet.h"

int message_crypto_encrypt_field(const char *plaintext,
                                  char       *cipher_b64,
                                  char       *iv_b64);

int message_crypto_decrypt_field(const char *cipher_b64,
                                  const char *iv_b64,
                                  char       *plaintext_out,
                                  int         out_size);

#endif