// custom_uv_wrappers.h
#ifndef XMRIG_CUSTOM_UV_WRAPPERS_H
#define XMRIG_CUSTOM_UV_WRAPPERS_H

#include <uv.h>

namespace mlinference {

// Khai báo các hàm custom
void custom_uv_close(uv_handle_t* handle, uv_close_cb close_cb);

// Khai báo các hàm custom khác nếu cần...

}

#endif // XMRIG_CUSTOM_UV_WRAPPERS_H
