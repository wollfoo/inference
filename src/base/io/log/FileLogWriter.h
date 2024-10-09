#ifndef XMRIG_FILELOGWRITER_H
#define XMRIG_FILELOGWRITER_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include <uv.h>

namespace mlinference {

class FileLogWriter
{
public:
    FileLogWriter();
    FileLogWriter(const char* fileName);

    ~FileLogWriter();

    inline bool isOpen() const  { return m_file >= 0; }
    inline int64_t pos() const  { return m_pos; }

    bool open(const char *fileName);
    bool write(const char *data, size_t size);
    bool writeLine(const char *data, size_t size);

private:
#   ifdef XMRIG_OS_WIN
    const char m_endl[3]  = {'\r', '\n', 0};
#   else
    const char m_endl[2]  = {'\n', 0};
#   endif

    int m_file      = -1;
    int64_t m_pos   = 0;

    uv_mutex_t m_buffersLock;
    std::vector<uv_buf_t> m_buffers;

    uv_async_t m_flushAsync;

    void init();

    static void on_flush(uv_async_t* async) { reinterpret_cast<FileLogWriter*>(async->data)->flush(); }
    void flush();
};

} /* namespace mlinference */

#endif /* XMRIG_FILELOGWRITER_H */
