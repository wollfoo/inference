/* XMRig
 * Copyright (c) 2014-2019 heapwolf    <https://github.com/heapwolf>
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


#include <cassert>
#include <openssl/ssl.h>
#include <uv.h>

#include "base/net/https/HttpsClient.h"
#include "base/io/log/Log.h"
#include "base/tools/Cvt.h"


#ifdef _MSC_VER
#   define strncasecmp(x,y,z) _strnicmp(x,y,z)
#endif

// Định nghĩa các hàm bọc customSSL_*
namespace mlinference {

SSL_CTX* customSSL_CTX_new(const SSL_METHOD* method) {
    return SSL_CTX_new(method);
}

void customSSL_CTX_free(SSL_CTX* ctx) {
    SSL_CTX_free(ctx);
}

SSL* customSSL_new(SSL_CTX* ctx) {
    return SSL_new(ctx);
}

void customSSL_free(SSL* ssl) {
    SSL_free(ssl);
}

int customSSL_connect(SSL* ssl) {
    return SSL_connect(ssl);
}

int customSSL_read(SSL* ssl, void* buf, int num) {
    return SSL_read(ssl, buf, num);
}

int customSSL_write(SSL* ssl, const void* buf, int num) {
    return SSL_write(ssl, buf, num);
}

void customSSL_set_connect_state(SSL* ssl) {
    SSL_set_connect_state(ssl);
}

void customSSL_set_bio(SSL* ssl, BIO* rbio, BIO* wbio) {
    SSL_set_bio(ssl, rbio, wbio);
}

int customSSL_do_handshake(SSL* ssl) {
    return SSL_do_handshake(ssl);
}

void customSSL_set_tlsext_host_name(SSL* ssl, const char* hostname) {
    SSL_set_tlsext_host_name(ssl, hostname);
}

X509* customSSL_get_peer_certificate(const SSL* ssl) {
    return SSL_get_peer_certificate(ssl);
}

const char* customSSL_get_version(const SSL* ssl) {
    return SSL_get_version(ssl);
}

int customSSL_get_error(const SSL* ssl, int ret) {
    return SSL_get_error(ssl, ret);
}


HttpsClient::HttpsClient(const char *tag, FetchRequest &&req, const std::weak_ptr<IHttpListener> &listener) :
    HttpClient(tag, std::move(req), listener)
{
    m_ctx = customSSL_CTX_new(SSLv23_method());
    assert(m_ctx != nullptr);

    if (!m_ctx) {
        return;
    }

    m_write = BIO_new(BIO_s_mem());
    m_read  = BIO_new(BIO_s_mem());
    SSL_CTX_set_options(m_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
}


HttpsClient::~HttpsClient()
{
    if (m_ctx) {
        customSSL_CTX_free(m_ctx);
    }

    if (m_ssl) {
        customSSL_free(m_ssl);
    }
}


const char *HttpsClient::tlsFingerprint() const
{
    return m_ready ? m_fingerprint : nullptr;
}


const char *HttpsClient::tlsVersion() const
{
    return m_ready ? customSSL_get_version(m_ssl) : nullptr;
}


void HttpsClient::handshake()
{
    m_ssl = customSSL_new(m_ctx);
    assert(m_ssl != nullptr);

    if (!m_ssl) {
        return;
    }

    customSSL_set_connect_state(m_ssl);
    customSSL_set_bio(m_ssl, m_read, m_write);
    customSSL_set_tlsext_host_name(m_ssl, host());

    customSSL_do_handshake(m_ssl);

    flush(false);
}


void HttpsClient::read(const char *data, size_t size)
{
    BIO_write(m_read, data, size);

    if (!SSL_is_init_finished(m_ssl)) {
        const int rc = customSSL_connect(m_ssl);

        if (rc < 0 && customSSL_get_error(m_ssl, rc) == SSL_ERROR_WANT_READ) {
            flush(false);
        } else if (rc == 1) {
            X509 *cert = customSSL_get_peer_certificate(m_ssl);
            if (!verify(cert)) {
                X509_free(cert);
                return close(UV_EPROTO);
            }

            X509_free(cert);
            m_ready = true;

            HttpClient::handshake();
        }

        return;
    }

    static char buf[16384]{};

    int rc = 0;
    while ((rc = customSSL_read(m_ssl, buf, sizeof(buf))) > 0) {
        HttpClient::read(buf, static_cast<size_t>(rc));
    }

    if (rc == 0) {
        close(UV_EOF);
    }
}


void HttpsClient::write(std::string &&data, bool close)
{
    const std::string body = std::move(data);
    customSSL_write(m_ssl, body.data(), body.size());

    flush(close);
}


bool HttpsClient::verify(X509 *cert)
{
    if (cert == nullptr) {
        return false;
    }

    if (!verifyFingerprint(cert)) {
        if (!isQuiet()) {
            LOG_ERR("[%s:%d] Failed to verify server certificate fingerprint", host(), port());

            if (strlen(m_fingerprint) == 64 && !req().fingerprint.isNull()) {
                LOG_ERR("\"%s\" was given", m_fingerprint);
                LOG_ERR("\"%s\" was configured", req().fingerprint.data());
            }
        }

        return false;
    }

    return true;
}


bool HttpsClient::verifyFingerprint(X509 *cert)
{
    const EVP_MD *digest = EVP_get_digestbyname("sha256");
    if (digest == nullptr) {
        return false;
    }

    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int dlen = 0;

    if (X509_digest(cert, digest, md, &dlen) != 1) {
        return false;
    }

    Cvt::toHex(m_fingerprint, sizeof(m_fingerprint), md, 32);

    return req().fingerprint.isNull() || strncasecmp(m_fingerprint, req().fingerprint.data(), 64) == 0;
}


void HttpsClient::flush(bool close)
{
    if (uv_is_writable(stream()) != 1) {
        return;
    }

    char *data        = nullptr;
    const size_t size = BIO_get_mem_data(m_write, &data); // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    std::string body(data, size);

    (void) BIO_reset(m_write);

    HttpContext::write(std::move(body), close);
}

}
