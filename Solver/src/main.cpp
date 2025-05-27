#include <iostream>
#include <thread>
#include <stdexcept>

#include "epsilon.h"
#include "matrix.h"
#include "bounded.h"
#include "../include/solver.h"
   // ← готовые AugmentedMatrix / Producer / Consumer
#include <vector>
#include <string>

using namespace std;

/*------------------------------------------------------------
 *  Пример функции-генератора двухдиагональной задачи
 *-----------------------------------------------------------*/
static AugmentedMatrix make_matrix(std::size_t n, const std::string& name)
{
    if (n < 2) throw std::runtime_error("n must be ≥ 2");

    auto A = std::make_unique< Matrix<double> >(n, n);
    auto B = std::make_unique< Matrix<double> >(n, 1);
    auto X = std::make_unique< Matrix<double> >(n, 1);   // заполним позже

    for (std::size_t i = 0; i < n; ++i) {
        (*A)(i, i) = 7.0 / 5.0;                // главная диагональ
        if (i + 1 < n) (*A)(i, i + 1) = 11.0 / 3.0;   // верхняя
    }

    for (std::size_t i = 0; i < n; ++i) {
        double k = static_cast<double>(i + 1);
        (*B)(i, 0) =
            (152.0 * k + 118.0) / (15.0 * (2.0 * k + 1.0) * (2.0 * k + 3.0));
        (*X)(i, 0) = 1.0 / (2.0 * k + 1.0);            // «истинное» решение
    }

    // последний элемент правой части корректируем по формуле
    (*B)(n - 1, 0) = 7.0 / (5.0 * (2.0 * (n - 1) + 1.0));
    (*X)(n - 1, 0) = 1.0 / (2.0 * (n - 1) + 1.0);

    AugmentedMatrix task;
    task.A = std::move(A);
    task.B = std::move(B);
    task.X = std::move(X);          // пока хранит «идеал» — удобно для теста
    task.Filename = name;

    return task;
}

int main(int argc, char* argv[])
{
    /*--------------------------------------------------------
     *  1. Тестируем шаблон solve() на маленькой 3×3 системе
     *-------------------------------------------------------*/
    Matrix<double> A(3, 3);
    A(0, 0) =  2;  A(0, 1) = -1;
    A(1, 1) =  3;  A(1, 2) =  1;
    A(2, 2) =  4;                      // остальное нули

    Matrix<double> b(3, 1);
    b(0, 0) = 3;
    b(1, 0) = 5;
    b(2, 0) = 8;

    Matrix<double> x(3, 1);

    try {
        solve(A, x, b);
        std::cout << "Решение 3×3:\n" << x << '\n';
    }
    catch (const std::exception& e) {
        std::cerr << "solve() error: " << e.what() << '\n';
    }

    /*--------------------------------------------------------
     *  2. Демонстрация Producer / Consumer на ~«файлах»
     *     (здесь вместо чтения из «Задачи» кладём в очередь
     *     несколько сгенерированных матриц).
     *-------------------------------------------------------*/
    Bounded<AugmentedMatrix> queue(10);

    // Параметры потоков
    const int num_consumers = (argc > 1 ? std::stoi(argv[1]) : 2);

    // --- ручной Producer: бросаем 5 задач в очередь ---
    for (std::size_t k = 1; k <= 5; ++k) {
        queue.push(make_matrix(10 + k, "gen" + std::to_string(k) + ".txt"));
    }
    // --- отправляем столько «пустых» задач, сколько consumers ---
    for (int i = 0; i < num_consumers; ++i) queue.push(AugmentedMatrix{});

    // --- запускаем Consumers ---
    std::vector<std::thread> workers;
    for (int i = 0; i < num_consumers; ++i)
        workers.emplace_back(Consumer(queue, 0));  // qtty не нужен

    for (auto& t : workers) t.join();

    return 0;
}
