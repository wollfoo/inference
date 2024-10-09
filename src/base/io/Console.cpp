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

#include "base/io/Console.h"
#include "base/kernel/interfaces/IConsoleListener.h"
#include "base/tools/Handle.h"


mlinference::Console::Console(IConsoleListener *listener)
    : m_listener(listener)
{
    if (!isSupported()) {
        return;
    }

    m_tty = new uv_tty_t;
    m_tty->data = this;
    custom_uv_tty_init(uv_default_loop(), m_tty, 0, 1);  // Use custom_uv_tty_init

    if (!uv_is_readable(reinterpret_cast<uv_stream_t*>(m_tty))) {
        return;
    }

    custom_uv_tty_set_mode(m_tty, UV_TTY_MODE_RAW);  // Use custom_uv_tty_set_mode
    custom_uv_read_start(reinterpret_cast<uv_stream_t*>(m_tty), Console::onAllocBuffer, Console::onRead);  // Use custom_uv_read_start
}


mlinference::Console::~Console()
{
    custom_uv_tty_reset_mode();  // Use custom_uv_tty_reset_mode

    custom_uv_close(reinterpret_cast<uv_handle_t*>(m_tty), nullptr);  // Use custom_uv_close
}


bool mlinference::Console::isSupported()
{
    const uv_handle_type type = custom_uv_guess_handle(0);  // Use custom_uv_guess_handle
    return type == UV_TTY || type == UV_NAMED_PIPE;
}


void mlinference::Console::onAllocBuffer(uv_handle_t *handle, size_t, uv_buf_t *buf)
{
    auto console = static_cast<Console*>(handle->data);
    buf->len  = 1;
    buf->base = console->m_buf;
}


void mlinference::Console::onRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    if (nread < 0) {
        return custom_uv_close(reinterpret_cast<uv_handle_t*>(stream), nullptr);  // Use custom_uv_close
    }

    if (nread == 1) {
        static_cast<Console*>(stream->data)->m_listener->onConsoleCommand(buf->base[0]);
    }
}
