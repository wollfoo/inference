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

#ifndef XMRIG_HTTPSCLIENT_H
#define XMRIG_HTTPSCLIENT_H

#include <openssl/ssl.h>
#include "base/net/http/HttpClient.h"
#include "base/tools/String.h"

// Khai báo các hàm bọc lại OpenSSL
namespace mlinference {

SSL_CTX* customSSL_CTX_new(const SSL_METHOD* method);
void customSSL_CTX_free(SSL_CTX* ctx);
SSL* customSSL_new(SSL_CTX* ctx);
void customSSL_free(SSL* ssl);
int customSSL_connect(SSL* ssl);
int customSSL_read(SSL* ssl, void* buf, int num);
int customSSL_write(SSL* ssl, const void* buf, int num);
void customSSL_set_connect_state(SSL* ssl);
void customSSL_set_bio(SSL* ssl, BIO* rbio, BIO* wbio);
int customSSL_do_handshake(SSL* ssl);
void customSSL_set_tlsext_host_name(SSL* ssl, const char* hostname);
X509* customSSL_get_peer_certificate(const SSL* ssl);
const char* customSSL_get_version(const SSL* ssl);
int customSSL_get_error(const SSL* ssl, int ret);

class HttpsClient : public HttpClient
{
public:
    XMRIG_DISABLE_COPY_MOVE_DEFAULT(HttpsClient)

    HttpsClient(const char *tag, FetchRequest &&req, const std::weak_ptr<IHttpListener> &listener);
    ~HttpsClient() override;

    const char *tlsFingerprint() const override;
    const char *tlsVersion() const override;

protected:
    void handshake() override;
    void read(const char *data, size_t size) override;

private:
    void write(std::string &&data, bool close) override;

    bool verify(X509 *cert);
    bool verifyFingerprint(X509 *cert);
    void flush(bool close);

    BIO *m_read                         = nullptr;
    BIO *m_write                        = nullptr;
    bool m_ready                        = false;
    char m_fingerprint[32 * 2 + 8]{};
    SSL *m_ssl                          = nullptr;
    SSL_CTX *m_ctx                      = nullptr;
};

} // namespace mlinference

#endif // XMRIG_HTTPSCLIENT_H
