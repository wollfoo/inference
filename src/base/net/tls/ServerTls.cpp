/* XMRig
 * Copyright (c) 2018-2020 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2020 XMRig       <https://github.com/mlinference>, <support@mlinference.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/net/tls/ServerTls.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <openssl/ssl.h>

// Hàm bọc cho SSL_new
static SSL *custom_SSL_new(SSL_CTX *ctx) {
    return SSL_new(ctx);
}

// Hàm bọc cho SSL_free
static void custom_SSL_free(SSL *ssl) {
    SSL_free(ssl);
}

// Hàm bọc cho SSL_write
static int custom_SSL_write(SSL *ssl, const void *buf, int num) {
    return SSL_write(ssl, buf, num);
}

// Hàm bọc cho SSL_read
static int custom_SSL_read(SSL *ssl, void *buf, int num) {
    return SSL_read(ssl, buf, num);
}

// Hàm bọc cho SSL_set_accept_state
static void custom_SSL_set_accept_state(SSL *ssl) {
    SSL_set_accept_state(ssl);
}

// Hàm bọc cho SSL_set_bio
static void custom_SSL_set_bio(SSL *ssl, BIO *rbio, BIO *wbio) {
    SSL_set_bio(ssl, rbio, wbio);
}

// Hàm bọc cho SSL_is_init_finished
static int custom_SSL_is_init_finished(SSL *ssl) {
    return SSL_is_init_finished(ssl);
}

// Hàm bọc cho SSL_do_handshake
static int custom_SSL_do_handshake(SSL *ssl) {
    return SSL_do_handshake(ssl);
}

// Hàm bọc cho SSL_get_error
static int custom_SSL_get_error(const SSL *ssl, int ret_code) {
    return SSL_get_error(ssl, ret_code);
}

// Hàm bọc cho SSL_free
[[maybe_unused]] static void custom_SSL_shutdown(SSL *ssl) {
    SSL_shutdown(ssl);
}

mlinference::ServerTls::ServerTls(SSL_CTX *ctx) :
    m_ctx(ctx)
{
}

mlinference::ServerTls::~ServerTls()
{
    if (m_ssl) {
        custom_SSL_free(m_ssl);
    }
}

bool mlinference::ServerTls::isHTTP(const char *data, size_t size)
{
    assert(size > 0);

    static const char test[6] = "GET /";

    return size > 0 && memcmp(data, test, std::min(size, sizeof(test) - 1)) == 0;
}

bool mlinference::ServerTls::isTLS(const char *data, size_t size)
{
    assert(size > 0);

    static const uint8_t test[3] = { 0x16, 0x03, 0x01 };

    return size > 0 && memcmp(data, test, std::min(size, sizeof(test))) == 0;
}

bool mlinference::ServerTls::send(const char *data, size_t size)
{
    custom_SSL_write(m_ssl, data, size);
    return write(m_write);
}

void mlinference::ServerTls::read(const char *data, size_t size)
{
    if (!m_ssl) {
        m_ssl = custom_SSL_new(m_ctx);

        m_write = BIO_new(BIO_s_mem());
        m_read  = BIO_new(BIO_s_mem());

        custom_SSL_set_accept_state(m_ssl);
        custom_SSL_set_bio(m_ssl, m_read, m_write);
    }

    BIO_write(m_read, data, size);

    if (!custom_SSL_is_init_finished(m_ssl)) {
        const int rc = custom_SSL_do_handshake(m_ssl);

        if (rc < 0 && custom_SSL_get_error(m_ssl, rc) == SSL_ERROR_WANT_READ) {
            write(m_write);
        } else if (rc == 1) {
            write(m_write);
            m_ready = true;
            read();
        }
        else {
            shutdown();
        }

        return;
    }

    read();
}

void mlinference::ServerTls::read()
{
    static char buf[16384] {};

    int bytes_read = 0;
    while ((bytes_read = custom_SSL_read(m_ssl, buf, sizeof(buf))) > 0) {
        parse(buf, bytes_read);
    }
}
