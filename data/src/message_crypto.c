

#include "message_crypto.h"
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

/*
 * Clé AES-256 partagée client/serveur.
 * En prod : charger depuis getenv("MSG_AES_KEY").
 * La même clé doit être sur le serveur pour déchiffrer à l'affichage.
 */
static const unsigned char AES_KEY[32] = {
    0x2b,0x7e,0x15,0x16, 0x28,0xae,0xd2,0xa6,
    0xab,0xf7,0x15,0x88, 0x09,0xcf,0x4f,0x3c,
    0x6d,0x11,0x25,0xb0, 0x98,0x76,0x54,0x32,
    0x10,0xfe,0xdc,0xba, 0x98,0x76,0x54,0x32
};

 

static void b64_encode(const unsigned char *in, int len, char *out)
{
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *mem = BIO_new(BIO_s_mem());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_push(b64, mem);
    BIO_write(b64, in, len);
    BIO_flush(b64);
    BUF_MEM *p;
    BIO_get_mem_ptr(b64, &p);
    memcpy(out, p->data, p->length);
    out[p->length] = '\0';
    BIO_free_all(b64);
}

static int b64_decode(const char *in, unsigned char *out)
{
    int n = (int)strlen(in);
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *mem = BIO_new_mem_buf(in, n);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_push(b64, mem);
    int len = BIO_read(b64, out, n);
    BIO_free_all(b64);
    return len;
}


int message_crypto_encrypt_field(const char *plaintext,
                                  char       *cipher_b64,  /* out ≥ 512  */
                                  char       *iv_b64)      /* out ≥ 32   */
{
    unsigned char iv[16];
    unsigned char cipher[PACKET_FIELD_SIZE + 32];
    int len = 0, ct_len = 0;

    if (RAND_bytes(iv, 16) != 1) {
        fprintf(stderr, "[crypto] RAND_bytes failed\n");
        return -1;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, AES_KEY, iv) != 1) goto err;
    if (EVP_EncryptUpdate(ctx, cipher, &len,
                          (const unsigned char *)plaintext,
                          (int)strlen(plaintext)) != 1)                        goto err;
    ct_len = len;
    if (EVP_EncryptFinal_ex(ctx, cipher + len, &len) != 1)                     goto err;
    ct_len += len;
    EVP_CIPHER_CTX_free(ctx);

    b64_encode(iv,     16,     iv_b64);
    b64_encode(cipher, ct_len, cipher_b64);
    return 0;

err:
    EVP_CIPHER_CTX_free(ctx);
    return -1;
}

int message_crypto_decrypt_field(const char *cipher_b64,
                                  const char *iv_b64,
                                  char       *plaintext_out,
                                  int         out_size)
{
    unsigned char cipher[PACKET_FIELD_SIZE + 32];
    unsigned char iv[16];
    unsigned char plain[PACKET_FIELD_SIZE];
    int len = 0, pt_len = 0;

    int ct_len = b64_decode(cipher_b64, cipher);
    int iv_len = b64_decode(iv_b64,     iv);

    if (ct_len <= 0 || iv_len != 16) {
        fprintf(stderr, "[crypto] b64_decode failed\n");
        return -1;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, AES_KEY, iv) != 1) goto err;
    if (EVP_DecryptUpdate(ctx, plain, &len, cipher, ct_len) != 1)            goto err;
    pt_len = len;
    if (EVP_DecryptFinal_ex(ctx, plain + len, &len) != 1)                    goto err;
    pt_len += len;
    EVP_CIPHER_CTX_free(ctx);

    if (pt_len >= out_size) return -1;
    memcpy(plaintext_out, plain, pt_len);
    plaintext_out[pt_len] = '\0';
    return 0;

err:
    EVP_CIPHER_CTX_free(ctx);
    return -1;
}