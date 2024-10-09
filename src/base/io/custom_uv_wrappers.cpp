// custom_uv_wrappers.cpp
#include <uv.h>

namespace mlinference {

// Custom wrapper for uv_close
void custom_uv_close(uv_handle_t* handle, uv_close_cb close_cb) {
    uv_close(handle, close_cb);  // Gọi hàm libuv gốc
}

// Các hàm custom khác (nếu có) nên đặt ở đây...
}
