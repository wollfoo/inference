/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018-2020 SChernykh   <https://github.com/SChernykh>
 * Copyright 2016-2020 XMRig       <https://github.com/mlinference>, <support@mlinference.com>
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

#include "base/kernel/interfaces/ITcpServerListener.h"
#include "base/net/tools/TcpServer.h"
#include "base/tools/Handle.h"
#include "base/tools/String.h"
#include "base/io/log/custom_uv_wrappers.h"  // Thêm đường dẫn tới custom_uv_wrappers.h để sử dụng các hàm bọc.

static const mlinference::String kLocalHost("127.0.0.1");


mlinference::TcpServer::TcpServer(const String &host, uint16_t port, ITcpServerListener *listener) :
    m_host(host.isNull() ? kLocalHost : host),
    m_listener(listener),
    m_port(port)
{
    assert(m_listener != nullptr);

    m_tcp = new uv_tcp_t;
    custom_uv_tcp_init(uv_default_loop(), m_tcp);  // Sử dụng custom_uv_tcp_init
    m_tcp->data = this;

    custom_uv_tcp_nodelay(m_tcp, 1);  // Sử dụng custom_uv_tcp_nodelay

    if (m_host.contains(":") && custom_uv_ip6_addr(m_host.data(), m_port, reinterpret_cast<sockaddr_in6 *>(&m_addr)) == 0) {
        m_version = 6;
    }
    else if (custom_uv_ip4_addr(m_host.data(), m_port, reinterpret_cast<sockaddr_in *>(&m_addr)) == 0) {
        m_version = 4;
    }
}


mlinference::TcpServer::~TcpServer()
{
    Handle::close(m_tcp);
}


int mlinference::TcpServer::bind()
{
    if (!m_version) {
        return UV_EAI_ADDRFAMILY;
    }

    custom_uv_tcp_bind(m_tcp, reinterpret_cast<const sockaddr*>(&m_addr), 0);  // Sử dụng custom_uv_tcp_bind

    const int rc = custom_uv_listen(reinterpret_cast<uv_stream_t*>(m_tcp), 511, TcpServer::onConnection);  // Sử dụng custom_uv_listen
    if (rc != 0) {
        return rc;
    }

    if (!m_port) {
        sockaddr_storage storage = {};
        int size = sizeof(storage);

        custom_uv_tcp_getsockname(m_tcp, reinterpret_cast<sockaddr*>(&storage), &size);  // Sử dụng custom_uv_tcp_getsockname

        m_port = ntohs(reinterpret_cast<sockaddr_in *>(&storage)->sin_port);
    }

    return m_port;
}


void mlinference::TcpServer::create(uv_stream_t *stream, int status)
{
    if (status < 0) {
        return;
    }

    m_listener->onConnection(stream, m_port);
}


void mlinference::TcpServer::onConnection(uv_stream_t *stream, int status)
{
    static_cast<TcpServer *>(stream->data)->create(stream, status);
}
