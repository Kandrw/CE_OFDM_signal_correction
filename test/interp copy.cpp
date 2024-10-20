#include <iostream>
#include <vector>
#include <stdexcept>

double linearInterpolation(const std::vector<double>& x, const std::vector<double>& y, double x_interp) {
    if (x.size() != y.size()) {
        throw std::invalid_argument("Vectors x and y must have the same size.");
    }

    // Проверка, что x_interp находится в пределах x
    if (x_interp < x.front() || x_interp > x.back()) {
        throw std::out_of_range("x_interp is out of the range of x.");
    }

    // Поиск интервала, в котором находится x_interp
    for (size_t i = 1; i < x.size(); ++i) {
        if (x[i] >= x_interp) {
            // Линейная интерполяция
            double x0 = x[i - 1];
            double x1 = x[i];
            double y0 = y[i - 1];
            double y1 = y[i];

            return y0 + (y1 - y0) * (x_interp - x0) / (x1 - x0);
        }
    }

    // Не должно быть достигнуто, если x_interp находится в пределах x
    throw std::runtime_error("Interpolation failed.");
}

int main() {
    // Пример данных 
    std::vector<double> x = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> y = {2.0, 3.0, 5.0, 7.0, 11.0};

    // Значение для интерполяции 
    double x_interp = 3.5;

    try {
        double result = linearInterpolation(x, y, x_interp);
        std::cout << "Interpolated value at x = " << x_interp << " is " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
