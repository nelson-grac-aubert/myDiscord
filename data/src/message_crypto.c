/*
 * data/message_crypto.c
 *
 * Appelé par message.c juste avant l'INSERT en base.
 * Branché sur le flux :
 *   ui_chat  →  chat_controller_handle_keydown / handle_left_click
 *            →  message_model_send()   (dans message.c)
 *            →  message_crypto_encrypt_and_insert()   ← ICI
 *            →  INSERT INTO message (encrypted_content, content_iv, ...)
 *
 * Compilation : ajouter à votre Makefile
 *   LDFLAGS += -lssl -lcrypto -lpq
 */

#include "message_crypto.h"
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <libpq-fe.h>

/* ── Clé AES-256 ────────────────────────────────────────────────────────
 * En production : charger depuis getenv("MSG_AES_KEY") ou un fichier
 * sécurisé. Ne jamais stocker en base.                                  */
static const unsigned char AES_KEY[32] = {
    0x2b,0x7e,0x15,0x16, 0x28,0xae,0xd2,0xa6,
    0xab,0xf7,0x15,0x88, 0x09,0xcf,0x4f,0x3c,
    0x6d,0x11,0x25,0xb0, 0x98,0x76,0x54,0x32,
    0x10,0xfe,0xdc,0xba, 0x98,0x76,0x54,0x32
};

/* ── Base64 ─────────────────────────────────────────────────────────── */

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

/* ── Chiffrement ────────────────────────────────────────────────────── */

/*
 * message_crypto_encrypt()
 *
 * Entrée  : plaintext   → texte tapé par l'utilisateur dans input_buffer
 * Sorties : iv_b64      → stocké dans  content_iv        (colonne DB)
 *           cipher_b64  → stocké dans  encrypted_content (colonne DB)
 * Retour  : longueur ciphertext brut, -1 si erreur
 */
int message_crypto_encrypt(const char *plaintext,
                            char       *iv_b64,
                            char       *cipher_b64)
{
    unsigned char iv[16];
    unsigned char cipher[4096];
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
                          (int)strlen(plaintext)) != 1)                   goto err;
    ct_len = len;
    if (EVP_EncryptFinal_ex(ctx, cipher + len, &len) != 1)                goto err;
    ct_len += len;
    EVP_CIPHER_CTX_free(ctx);

    b64_encode(iv,     16,     iv_b64);
    b64_encode(cipher, ct_len, cipher_b64);
    return ct_len;

err:
    EVP_CIPHER_CTX_free(ctx);
    return -1;
}

/* ── Déchiffrement ──────────────────────────────────────────────────── */

/*
 * message_crypto_decrypt()
 *
 * Entrées : cipher_b64  ← encrypted_content lu en base
 *           iv_b64      ← content_iv         lu en base
 * Sortie  : plaintext_out → texte affiché dans draw_chat_messages()
 * Retour  : longueur plaintext, -1 si erreur
 */
int message_crypto_decrypt(const char *cipher_b64,
                            const char *iv_b64,
                            char       *plaintext_out,
                            int         out_size)
{
    unsigned char cipher[4096];
    unsigned char iv[16];
    unsigned char plain[4096];
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
    return pt_len;

err:
    EVP_CIPHER_CTX_free(ctx);
    return -1;
}

/* ── INSERT chiffré en base ─────────────────────────────────────────── */

/*
 * message_crypto_encrypt_and_insert()
 *
 * Remplace l'INSERT direct dans message.c.
 * Appeler ainsi depuis message_model_send() :
 *
 *   // Avant (non chiffré) :
 *   // PQexecParams(conn, "INSERT INTO message ...", ...)
 *
 *   // Après (chiffré) :
 *   message_crypto_encrypt_and_insert(conn,
 *                                     layout->input_buffer,
 *                                     current_user.id,
 *                                     active_channel->id);
 *
 * Retour : 0 OK | -1 erreur crypto | -2 erreur DB
 */
int message_crypto_encrypt_and_insert(PGconn     *conn,
                                       const char *plaintext,
                                       int         id_author,
                                       int         id_channel)
{
    char iv_b64[64];
    char cipher_b64[6000];

    /* 1. Chiffrement du texte venant de input_buffer */
    if (message_crypto_encrypt(plaintext, iv_b64, cipher_b64) < 0) {
        fprintf(stderr, "[crypto] encrypt failed — message non envoyé\n");
        return -1;
    }

    /* 2. Paramètres SQL */
    char author_s[16], channel_s[16];
    snprintf(author_s,  sizeof(author_s),  "%d", id_author);
    snprintf(channel_s, sizeof(channel_s), "%d", id_channel);

    const char *params[4] = { cipher_b64, iv_b64, author_s, channel_s };

    /* 3. INSERT → table message */
    PGresult *res = PQexecParams(
        conn,
        "INSERT INTO message "
        "(encrypted_content, content_iv, id_author, id_channel) "
        "VALUES ($1, $2, $3, $4)",
        4, NULL, params, NULL, NULL, 0
    );

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "[db] INSERT failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -2;
    }

    PQclear(res);
    return 0;
}