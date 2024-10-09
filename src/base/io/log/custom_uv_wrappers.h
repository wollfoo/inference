#ifndef CUSTOM_UV_WRAPPERS_H
#define CUSTOM_UV_WRAPPERS_H

#include <uv.h>

namespace mlinference {

// Function wrappers for libuv

inline int custom_uv_tty_init(uv_loop_t* loop, uv_tty_t* tty, uv_file fd, int readable) {
    return uv_tty_init(loop, tty, fd, readable);
}

inline int custom_uv_tty_set_mode(uv_tty_t* tty, uv_tty_mode_t mode) {
    return uv_tty_set_mode(tty, mode);
}

inline uv_handle_type custom_uv_guess_handle(uv_file file) {
    return uv_guess_handle(file);
}

// Wrapper for uv_fs_event_init
inline void custom_uv_fs_event_init(uv_loop_t* loop, uv_fs_event_t* handle) {
    uv_fs_event_init(loop, handle);
}

// Wrapper for uv_fs_event_start
inline void custom_uv_fs_event_start(uv_fs_event_t* handle, uv_fs_event_cb cb, const char* path, unsigned int flags) {
    uv_fs_event_start(handle, cb, path, flags);
}

// Wrapper for uv_fs_event_stop
inline void custom_uv_fs_event_stop(uv_fs_event_t* handle) {
    uv_fs_event_stop(handle);
}

// Wrapper for uv_close
inline void custom_uv_close(uv_handle_t* handle, uv_close_cb close_cb) {
    uv_close(handle, close_cb);
}

// Wrapper for uv_mutex_init
inline void custom_uv_mutex_init(uv_mutex_t* mutex) {
    uv_mutex_init(mutex);
}

// Wrapper for uv_mutex_destroy
inline void custom_uv_mutex_destroy(uv_mutex_t* mutex) {
    uv_mutex_destroy(mutex);
}

// Wrapper for uv_mutex_lock
inline void custom_uv_mutex_lock(uv_mutex_t* mutex) {
    uv_mutex_lock(mutex);
}

// Wrapper for uv_mutex_unlock
inline void custom_uv_mutex_unlock(uv_mutex_t* mutex) {
    uv_mutex_unlock(mutex);
}

// Wrapper for uv_async_init
inline void custom_uv_async_init(uv_loop_t* loop, uv_async_t* async, uv_async_cb async_cb) {
    uv_async_init(loop, async, async_cb);
}

// Wrapper for uv_async_send
inline void custom_uv_async_send(uv_async_t* async) {
    uv_async_send(async);
}

// Wrapper for uv_fs_open
inline int custom_uv_fs_open(uv_loop_t* loop, uv_fs_t* req, const char* path, int flags, int mode, uv_fs_cb cb) {
    return uv_fs_open(loop, req, path, flags, mode, cb);
}

// Wrapper for uv_fs_req_cleanup
inline void custom_uv_fs_req_cleanup(uv_fs_t* req) {
    uv_fs_req_cleanup(req);
}

// Wrapper for uv_fs_stat
inline int custom_uv_fs_stat(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb) {
    return uv_fs_stat(loop, req, path, cb);
}

// Wrapper for uv_fs_write
inline int custom_uv_fs_write(uv_loop_t* loop, uv_fs_t* req, uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset, uv_fs_cb cb) {
    return uv_fs_write(loop, req, file, bufs, nbufs, offset, cb);
}

// Wrapper for uv_strerror
inline const char* custom_uv_strerror(int err) {
    return uv_strerror(err);
}

// Wrapper for uv_read_start
inline int custom_uv_read_start(uv_stream_t* stream, uv_alloc_cb alloc_cb, uv_read_cb read_cb) {
    return uv_read_start(stream, alloc_cb, read_cb);
}

// Wrapper for uv_tty_reset_mode
inline void custom_uv_tty_reset_mode() {
    uv_tty_reset_mode();
}

// Wrapper for uv_poll_init
inline int custom_uv_poll_init(uv_loop_t* loop, uv_poll_t* handle, int fd) {
    return uv_poll_init(loop, handle, fd);
}

// Wrapper for uv_poll_start
inline int custom_uv_poll_start(uv_poll_t* handle, int events, uv_poll_cb cb) {
    return uv_poll_start(handle, events, cb);
}

// Wrapper for uv_translate_sys_error
inline int custom_uv_translate_sys_error(int sys_errno) {
    return uv_translate_sys_error(sys_errno);
}

// Wrapper for uv_tcp_init
inline int custom_uv_tcp_init(uv_loop_t* loop, uv_tcp_t* handle) {
    return uv_tcp_init(loop, handle);
}

// Wrapper for uv_tcp_nodelay
inline int custom_uv_tcp_nodelay(uv_tcp_t* handle, int enable) {
    return uv_tcp_nodelay(handle, enable);
}

// Wrapper for uv_is_writable
inline int custom_uv_is_writable(const uv_stream_t* stream) {
    return uv_is_writable(stream);
}

// Wrapper for uv_write
inline int custom_uv_write(uv_write_t* req, uv_stream_t* handle, const uv_buf_t bufs[], unsigned int nbufs, uv_write_cb cb) {
    return uv_write(req, handle, bufs, nbufs, cb);
}

// Wrapper for uv_tcp_getpeername
inline int custom_uv_tcp_getpeername(const uv_tcp_t* handle, struct sockaddr* name, int* namelen) {
    return uv_tcp_getpeername(handle, name, namelen);
}

// Wrapper for uv_ip4_name
inline int custom_uv_ip4_name(const struct sockaddr_in* src, char* dst, size_t size) {
    return uv_ip4_name(src, dst, size);
}

// Wrapper for uv_ip6_name
inline int custom_uv_ip6_name(const struct sockaddr_in6* src, char* dst, size_t size) {
    return uv_ip6_name(src, dst, size);
}

// Wrapper for uv_is_closing
inline int custom_uv_is_closing(const uv_handle_t* handle) {
    return uv_is_closing(handle);
}

// Wrapper for uv_ip4_addr
inline int custom_uv_ip4_addr(const char* ip, int port, struct sockaddr_in* addr) {
    return uv_ip4_addr(ip, port, addr);
}

// Wrapper for uv_ip6_addr
inline int custom_uv_ip6_addr(const char* ip, int port, struct sockaddr_in6* addr) {
    return uv_ip6_addr(ip, port, addr);
}

// Wrapper for uv_tcp_bind
inline int custom_uv_tcp_bind(uv_tcp_t* handle, const struct sockaddr* addr, unsigned int flags) {
    return uv_tcp_bind(handle, addr, flags);
}

// Wrapper for uv_listen
inline int custom_uv_listen(uv_stream_t* stream, int backlog, uv_connection_cb cb) {
    return uv_listen(stream, backlog, cb);
}

// Wrapper for uv_tcp_getsockname
inline int custom_uv_tcp_getsockname(const uv_tcp_t* handle, struct sockaddr* name, int* namelen) {
    return uv_tcp_getsockname(handle, name, namelen);
}

// Wrapper for uv_tcp_connect
inline int custom_uv_tcp_connect(uv_connect_t* req, uv_tcp_t* handle, const struct sockaddr* addr, uv_connect_cb cb) {
    return uv_tcp_connect(req, handle, addr, cb);
}

// Wrapper for uv_tcp_keepalive
inline int custom_uv_tcp_keepalive(uv_tcp_t* handle, int enable, unsigned int delay) {
    return uv_tcp_keepalive(handle, enable, delay);
}

// Wrapper for uv_try_write
inline int custom_uv_try_write(uv_stream_t* handle, const uv_buf_t bufs[], unsigned int nbufs) {
    return uv_try_write(handle, bufs, nbufs);
}

// Wrapper for uv_loop_delete
inline void custom_uv_loop_delete(uv_loop_t* loop) {
    uv_loop_close(loop);
    delete loop;
}

// Wrapper for uv_dlopen
inline int custom_uv_dlopen(const char *filename, uv_lib_t *lib) {
    return uv_dlopen(filename, lib);
}

// Wrapper for uv_dlclose
inline void custom_uv_dlclose(uv_lib_t *lib) {
    uv_dlclose(lib);
}

// Wrapper for uv_dlerror
inline const char *custom_uv_dlerror(const uv_lib_t *lib) {
    return uv_dlerror(lib);
}

// Wrapper for uv_dlsym
inline int custom_uv_dlsym(uv_lib_t *lib, const char *name, void **ptr) {
    return uv_dlsym(lib, name, ptr);
}

} // namespace mlinference

#endif // CUSTOM_UV_WRAPPERS_H
