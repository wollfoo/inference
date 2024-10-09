/* XMRig
 * Copyright (c) 2018-2021 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2021 XMRig       <https://github.com/mlinference>, <support@mlinference.com>
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


#include "base/io/log/FileLogWriter.h"
#include "base/io/Env.h"
#include "base/io/log/custom_uv_wrappers.h"  // Include custom_uv_wrappers.h for wrapping libuv functions

#include <cassert>

namespace mlinference {


static void fsWriteCallback(uv_fs_t *req)
{
    delete [] static_cast<char *>(req->data);

    custom_uv_fs_req_cleanup(req);  // Use custom_uv_fs_req_cleanup
    delete req;
}


} // namespace mlinference


mlinference::FileLogWriter::FileLogWriter()
{
    init();
}

mlinference::FileLogWriter::FileLogWriter(const char* fileName)
{
    init();
    open(fileName);
}

mlinference::FileLogWriter::~FileLogWriter()
{
    custom_uv_close(reinterpret_cast<uv_handle_t*>(&m_flushAsync), nullptr);  // Use custom_uv_close

    custom_uv_mutex_destroy(&m_buffersLock);  // Use custom_uv_mutex_destroy
}

void mlinference::FileLogWriter::init()
{
    custom_uv_mutex_init(&m_buffersLock);  // Use custom_uv_mutex_init

    custom_uv_async_init(uv_default_loop(), &m_flushAsync, on_flush);  // Use custom_uv_async_init
    m_flushAsync.data = this;
}

bool mlinference::FileLogWriter::open(const char *fileName)
{
    assert(fileName != nullptr);
    if (!fileName) {
        return false;
    }

    uv_fs_t req{};
    const auto path = Env::expand(fileName);  // path is of type mlinference::String
    m_file = custom_uv_fs_open(uv_default_loop(), &req, path.data(), O_CREAT | O_WRONLY, 0644, nullptr);  // Use path.data() for mlinference::String

    if (req.result < 0 || !isOpen()) {
        custom_uv_fs_req_cleanup(&req);  // Use custom_uv_fs_req_cleanup
        m_file = -1;

        return false;
    }

    custom_uv_fs_req_cleanup(&req);  // Use custom_uv_fs_req_cleanup

    custom_uv_fs_stat(uv_default_loop(), &req, path.data(), nullptr);  // Use path.data() for mlinference::String
    m_pos = req.statbuf.st_size;
    custom_uv_fs_req_cleanup(&req);  // Use custom_uv_fs_req_cleanup

    return true;
}


bool mlinference::FileLogWriter::write(const char *data, size_t size)
{
    if (!isOpen()) {
        return false;
    }

    uv_buf_t buf = uv_buf_init(new char[size], size);
    memcpy(buf.base, data, size);

    custom_uv_mutex_lock(&m_buffersLock);  // Use custom_uv_mutex_lock

    m_buffers.emplace_back(buf);
    custom_uv_async_send(&m_flushAsync);  // Use custom_uv_async_send

    custom_uv_mutex_unlock(&m_buffersLock);  // Use custom_uv_mutex_unlock

    return true;
}


bool mlinference::FileLogWriter::writeLine(const char *data, size_t size)
{
    if (!isOpen()) {
        return false;
    }

    constexpr size_t N = sizeof(m_endl) - 1;

    uv_buf_t buf = uv_buf_init(new char[size + N], size + N);
    memcpy(buf.base, data, size);
    memcpy(buf.base + size, m_endl, N);

    custom_uv_mutex_lock(&m_buffersLock);  // Use custom_uv_mutex_lock

    m_buffers.emplace_back(buf);
    custom_uv_async_send(&m_flushAsync);  // Use custom_uv_async_send

    custom_uv_mutex_unlock(&m_buffersLock);  // Use custom_uv_mutex_unlock

    return true;
}

void mlinference::FileLogWriter::flush()
{
    custom_uv_mutex_lock(&m_buffersLock);  // Use custom_uv_mutex_lock

    for (uv_buf_t buf : m_buffers) {
        uv_fs_t* req = new uv_fs_t;
        req->data = buf.base;

        custom_uv_fs_write(uv_default_loop(), req, m_file, &buf, 1, m_pos, fsWriteCallback);  // Use custom_uv_fs_write
        m_pos += buf.len;
    }
    m_buffers.clear();

    custom_uv_mutex_unlock(&m_buffersLock);  // Use custom_uv_mutex_unlock
}
