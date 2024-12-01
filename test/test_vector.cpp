#include <vector>
#include <iostream>
#include <algorithm> // Для std::copy






int main1() {

    std::vector<int> v1;
    int N = 20;
    for(int i = 0; i < N; ++i) {
        v1.push_back(i);
    }
    for(int i = 0; i < v1.size(); ++i) {
        std::cout<<v1[i]<<" ";
    }std::cout<<"\n";

    v1.erase(v1.begin() + 10, v1.begin() + 11);

    for(int i = 0; i < v1.size(); ++i) {
        std::cout<<v1[i]<<" ";
    }std::cout<<"\n";
    

}


int main() {
    // Исходный вектор
    std::vector<int> source = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // Вектор для копирования части
    std::vector<int> destination;

    // Указываем диапазон для копирования (например, с 2-го до 5-го элемента)
    auto start = source.begin() + 2; // 3-й элемент (индекс 2)
    auto end = source.begin() + 6;    // 6-й элемент (индекс 5, не включается)

    // Копируем часть вектора
    std::copy(start, end, std::back_inserter(destination));

    // Выводим результат
    for (int num : destination) {
        std::cout << num << " ";
    }
    std::cout<<"\n";

    return 0;
}
