#include <iostream>
#include <vector>
#include <complex>
#include <stdexcept>

std::vector<std::complex<double>> linearInterpolation(const std::vector<std::complex<double>>& points, size_t num_points) {
    if (points.size() < 2) {
        throw std::invalid_argument("At least two points are required for interpolation.");
    }

    std::vector<std::complex<double>> interpolated;
    interpolated.reserve(num_points);

    // Определяем шаг для интерполяции
    double step = static_cast<double>(points.size() - 1) / (num_points - 1);

    for (size_t i = 0; i < num_points; ++i) {
        double index = i * step;
        size_t lower_index = static_cast<size_t>(index);
        size_t upper_index = lower_index + 1;

        // Если мы находимся на границе, просто добавляем точку
        if (upper_index >= points.size()) {
            interpolated.push_back(points.back());
            continue;
        }

        // Линейная интерполяция
        double t = index - lower_index;
        std::complex<double> interpolated_value = points[lower_index] + t * (points[upper_index] - points[lower_index]);
        interpolated.push_back(interpolated_value);
    }

    return interpolated;
}

int main() {
    std::vector<std::complex<double>> points = {
        {1.0, 1.0},   // 1 + 1i
        {4.0, 4.0},   // 4 + 4i 
        {1.0, 4.0}    // 8 + 8i
    };

    size_t num_points = 16;

    try {
        std::vector<std::complex<double>> interpolated = linearInterpolation(points, num_points);

        // Вывод интерполированных значений
        for (const auto& value : interpolated) {
            // std::cout << value << " ";
            std::cout << value.real()<<" + "<<value.imag() << "j,\n";
            
        }
        std::cout << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
