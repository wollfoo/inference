// Không sử dụng hwloc.h và stdio.h

// Khai báo các thành phần của hwloc mà không cần file hwloc.h
HWLOC_DECLSPEC extern const struct hwloc_component hwloc_noos_component;
HWLOC_DECLSPEC extern const struct hwloc_component hwloc_xml_component;
HWLOC_DECLSPEC extern const struct hwloc_component hwloc_synthetic_component;
HWLOC_DECLSPEC extern const struct hwloc_component hwloc_xml_nolibxml_component;
HWLOC_DECLSPEC extern const struct hwloc_component hwloc_windows_component;
HWLOC_DECLSPEC extern const struct hwloc_component hwloc_x86_component;

// Hàm khởi tạo các thành phần hwloc
void initialize_hwloc_components() {
    // Mảng chứa các thành phần hwloc
    static const struct hwloc_component * hwloc_static_components[] = {
        &hwloc_noos_component,
        &hwloc_xml_component,
        &hwloc_synthetic_component,
        &hwloc_xml_nolibxml_component,
        &hwloc_windows_component,
        &hwloc_x86_component,
        NULL
    };

    // Giả sử việc khởi tạo thành công mà không cần in ra màn hình
}

int main() {
    // Khởi tạo các thành phần hwloc
    initialize_hwloc_components();
    return 0;
}
