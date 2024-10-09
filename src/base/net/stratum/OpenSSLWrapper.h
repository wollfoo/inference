#ifndef OPENSSL_WRAPPER_H
#define OPENSSL_WRAPPER_H

#include <openssl/ssl.h>
#include <openssl/err.h>

// Bọc hàm OpenSSL SSL_CTX_new
SSL_CTX* customSSL_CTX_new(const SSL_METHOD* method) {
    return SSL_CTX_new(method);
}

// Bọc hàm OpenSSL SSL_new
SSL* customSSL_new(SSL_CTX* ctx) {
    return SSL_new(ctx);
}

// Bọc hàm OpenSSL SSL_set_tlsext_host_name
int customSSL_set_tlsext_host_name(SSL* ssl, const char* name) {
    return SSL_set_tlsext_host_name(ssl, name);
}

// Bọc hàm OpenSSL SSL_set_connect_state
void customSSL_set_connect_state(SSL* ssl) {
    SSL_set_connect_state(ssl);
}

// Bọc hàm OpenSSL SSL_set_bio
void customSSL_set_bio(SSL* ssl, BIO* read_bio, BIO* write_bio) {
    SSL_set_bio(ssl, read_bio, write_bio);
}

// Bọc hàm OpenSSL SSL_do_handshake
int customSSL_do_handshake(SSL* ssl) {
    return SSL_do_handshake(ssl);
}

// Bọc hàm OpenSSL SSL_write
int customSSL_write(SSL* ssl, const void* buf, int num) {
    return SSL_write(ssl, buf, num);
}

// Bọc hàm OpenSSL SSL_get_version
const char* customSSL_get_version(const SSL* ssl) {
    return SSL_get_version(ssl);
}

// Bọc hàm OpenSSL SSL_connect
int customSSL_connect(SSL* ssl) {
    return SSL_connect(ssl);
}

// Bọc hàm OpenSSL SSL_get_error
int customSSL_get_error(const SSL* ssl, int ret_code) {
    return SSL_get_error(ssl, ret_code);
}

// Bọc hàm OpenSSL SSL_get_peer_certificate
X509* customSSL_get_peer_certificate(const SSL* ssl) {
    return SSL_get_peer_certificate(ssl);
}

// Bọc hàm OpenSSL SSL_read
int customSSL_read(SSL* ssl, void* buf, int num) {
    return SSL_read(ssl, buf, num);
}

// Bọc hàm OpenSSL SSL_free
void customSSL_free(SSL* ssl) {
    SSL_free(ssl);
}

// Bọc hàm OpenSSL SSL_CTX_free
void customSSL_CTX_free(SSL_CTX* ctx) {
    SSL_CTX_free(ctx);
}

// Thêm các hàm bọc khác nếu cần thiết

#endif // OPENSSL_WRAPPER_H
