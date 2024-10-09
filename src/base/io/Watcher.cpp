/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018-2019 SChernykh   <https://github.com/SChernykh>
 * Copyright 2016-2019 XMRig       <https://github.com/mlinference>, <support@mlinference.com>
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

#include <uv.h>

#include "base/kernel/interfaces/IWatcherListener.h"
#include "base/io/Watcher.h"
#include "base/tools/Handle.h"
#include "base/tools/Timer.h"
#include "base/io/log/custom_uv_wrappers.h"  // Include for custom UV wrappers

mlinference::Watcher::Watcher(const String &path, IWatcherListener *listener) :
    m_listener(listener),
    m_path(path)
{
    m_timer = new Timer(this);

    m_fsEvent = new uv_fs_event_t;
    m_fsEvent->data = this;
    custom_uv_fs_event_init(uv_default_loop(), m_fsEvent);  // Replace uv_fs_event_init with custom_uv_fs_event_init

    start();
}

mlinference::Watcher::~Watcher()
{
    delete m_timer;

    custom_uv_close(reinterpret_cast<uv_handle_t*>(m_fsEvent), nullptr);  // Use custom_uv_close
}

void mlinference::Watcher::onFsEvent(uv_fs_event_t *handle, const char *filename, int, int)
{
    if (!filename) {
        return;
    }

    static_cast<Watcher *>(handle->data)->queueUpdate();
}

void mlinference::Watcher::queueUpdate()
{
    m_timer->stop();
    m_timer->start(kDelay, 0);
}

void mlinference::Watcher::reload()
{
    m_listener->onFileChanged(m_path);

#   ifndef _WIN32
    custom_uv_fs_event_stop(m_fsEvent);  // Replace uv_fs_event_stop with custom_uv_fs_event_stop
    start();
#   endif
}

void mlinference::Watcher::start()
{
    custom_uv_fs_event_start(m_fsEvent, mlinference::Watcher::onFsEvent, m_path.data(), 0);  // Replace uv_fs_event_start with custom_uv_fs_event_start
}
