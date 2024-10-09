/* XMRig
 * Copyright (c) 2018-2023 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2023 XMRig       <https://github.com/mlinference>, <support@mlinference.com>
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

#include "base/kernel/Platform.h"

#include <cstring>
#include <uv.h>

#ifdef XMRIG_FEATURE_TLS
#   include <openssl/ssl.h>
#   include <openssl/err.h>

// Bọc các hàm OpenSSL
void custom_SSL_library_init() {
    SSL_library_init();
}

void custom_SSL_load_error_strings() {
    SSL_load_error_strings();
}

void custom_ERR_load_BIO_strings() {
#if OPENSSL_VERSION_NUMBER < 0x30000000L
    ERR_load_BIO_strings();
#endif
}

void custom_ERR_load_crypto_strings() {
    ERR_load_crypto_strings();
}

void custom_OpenSSL_add_all_digests() {
    OpenSSL_add_all_digests();
}
#endif

namespace mlinference {

String Platform::m_userAgent;

} // namespace mlinference

void mlinference::Platform::init(const char *userAgent)
{
#ifdef XMRIG_FEATURE_TLS
    custom_SSL_library_init();
    custom_SSL_load_error_strings();

#if OPENSSL_VERSION_NUMBER < 0x30000000L || defined(LIBRESSL_VERSION_NUMBER)
    custom_ERR_load_BIO_strings();
    custom_ERR_load_crypto_strings();
#endif

    custom_OpenSSL_add_all_digests();
#endif

    if (userAgent) {
        m_userAgent = userAgent;
    }
    else {
        m_userAgent = createUserAgent();
    }
}
