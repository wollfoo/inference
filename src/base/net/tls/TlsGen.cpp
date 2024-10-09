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

#include "base/net/tls/TlsGen.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdexcept>
#include <fstream>

namespace mlinference {

static const char *kLocalhost = "localhost";

// Bọc hàm OpenSSL để tạo khóa
static EVP_PKEY *custom_generate_pkey() {
#   if OPENSSL_VERSION_NUMBER < 0x30000000L || defined(LIBRESSL_VERSION_NUMBER)
    auto pkey = EVP_PKEY_new();
    if (!pkey) {
        return nullptr;
    }

    auto exponent = BN_new();
    auto rsa      = RSA_new();

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    if (!exponent || !rsa || !BN_set_word(exponent, RSA_F4) || 
        !RSA_generate_key_ex(rsa, 2048, exponent, nullptr) || 
        !EVP_PKEY_assign_RSA(pkey, rsa)) {
        EVP_PKEY_free(pkey);
        BN_free(exponent);
        RSA_free(rsa);
        return nullptr;
    }

    BN_free(exponent);
    return pkey;
#   else
    return EVP_RSA_gen(2048);
#   endif
}

// Kiểm tra tệp có tồn tại hay không
bool isFileExist(const char *fileName) {
    std::ifstream in(fileName);
    return in.good();
}

// Định nghĩa các hàm bọc OpenSSL
EVP_PKEY *custom_EVP_PKEY_new() {
    return EVP_PKEY_new();
}

void custom_EVP_PKEY_free(EVP_PKEY *pkey) {
    EVP_PKEY_free(pkey);
}

X509 *custom_X509_new() {
    return X509_new();
}

void custom_X509_free(X509 *x509) {
    X509_free(x509);
}

bool custom_X509_set_pubkey(X509 *x509, EVP_PKEY *pkey) {
    return X509_set_pubkey(x509, pkey);
}

ASN1_INTEGER *custom_X509_get_serialNumber(X509 *x509) {
    return X509_get_serialNumber(x509);
}

void custom_X509_gmtime_adj(X509 *x509, long adj) {
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 315360000L);
}

X509_NAME *custom_X509_get_subject_name(X509 *x509) {
    return X509_get_subject_name(x509);
}

bool custom_X509_add_entry_by_txt(X509_NAME *name, const char *field, int type, const uint8_t *value, int len, int loc, int opt) {
    return X509_NAME_add_entry_by_txt(name, field, type, value, len, loc, opt);
}

void custom_X509_set_issuer_name(X509 *x509, X509_NAME *name) {
    X509_set_issuer_name(x509, name);
}

int custom_X509_sign(X509 *x509, EVP_PKEY *pkey, const EVP_MD *type) {
    return X509_sign(x509, pkey, type);
}

bool custom_PEM_write_PrivateKey(FILE *bp, EVP_PKEY *x, const EVP_CIPHER *enc, unsigned char *kstr, int klen, pem_password_cb *cb, void *u) {
    return PEM_write_PrivateKey(bp, x, enc, kstr, klen, cb, u);
}

bool custom_PEM_write_X509(FILE *bp, X509 *x) {
    return PEM_write_X509(bp, x);
}

// Destructor của TlsGen
TlsGen::~TlsGen() {
    custom_EVP_PKEY_free(m_pkey);
    custom_X509_free(m_x509);
}

// Phương thức generate để tạo chứng chỉ
void TlsGen::generate(const char *commonName) {
    if (isFileExist(m_cert) && isFileExist(m_certKey)) {
        return;
    }

    m_pkey = custom_generate_pkey();
    if (!m_pkey) {
        throw std::runtime_error("RSA key generation failed.");
    }

    if (!generate_x509(commonName == nullptr || strlen(commonName) == 0 ? kLocalhost : commonName)) {
        throw std::runtime_error("x509 certificate generation failed.");
    }

    if (!write()) {
        throw std::runtime_error("unable to write certificate to disk.");
    }
}

// Phương thức generate_x509 để tạo x509 certificate
bool TlsGen::generate_x509(const char *commonName) {
    m_x509 = custom_X509_new();
    if (!m_x509) {
        return false;
    }

    if (!custom_X509_set_pubkey(m_x509, m_pkey)) {
        return false;
    }

    ASN1_INTEGER_set(custom_X509_get_serialNumber(m_x509), 1);
    custom_X509_gmtime_adj(m_x509, 0);
    custom_X509_gmtime_adj(m_x509, 315360000L);

    auto name = custom_X509_get_subject_name(m_x509);
    custom_X509_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<const uint8_t *>(commonName), -1, -1, 0);
    custom_X509_set_issuer_name(m_x509, name);

    return custom_X509_sign(m_x509, m_pkey, EVP_sha256());
}

// Phương thức write để ghi chứng chỉ vào tệp
bool TlsGen::write() {
    auto pkey_file = fopen(m_certKey, "wb");
    if (!pkey_file) {
        return false;
    }

    bool ret = custom_PEM_write_PrivateKey(pkey_file, m_pkey, nullptr, nullptr, 0, nullptr, nullptr);
    fclose(pkey_file);

    if (!ret) {
        return false;
    }

    auto x509_file = fopen(m_cert, "wb");
    if (!x509_file) {
        return false;
    }

    ret = custom_PEM_write_X509(x509_file, m_x509);
    fclose(x509_file);

    return ret;
}

} // namespace mlinference
