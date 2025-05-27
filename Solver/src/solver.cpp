#include "solver.h"
#include "matrix.h"

#include <atomic>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <cmath>
#include <filesystem>      // только для создания директории «Решения»

// -------------------------------------------------------
//  Глобальные статические переменные (инициализация)
// -------------------------------------------------------
std::atomic<int> Producer::active_producers{0};
int              Producer::total_consumers     = 0;
std::atomic<int> Consumer::total_consumers{0};

// -------------------------------------------------------
//  AugmentedMatrix: реализации конструкторов / присв.
// -------------------------------------------------------
AugmentedMatrix::AugmentedMatrix()
    : A(nullptr), B(nullptr), X(nullptr) {}

AugmentedMatrix::AugmentedMatrix(AugmentedMatrix&& other) noexcept
    : A(std::move(other.A)),
      B(std::move(other.B)),
      X(std::move(other.X)),
      Filename(std::move(other.Filename)) {}

AugmentedMatrix&
AugmentedMatrix::operator=(AugmentedMatrix&& other) noexcept {
    if (this != &other) {
        A        = std::move(other.A);
        B        = std::move(other.B);
        X        = std::move(other.X);
        Filename = std::move(other.Filename);
    }
    return *this;
}

// -------------------------------------------------------
//  Producer
// -------------------------------------------------------
Producer::Producer(Bounded<AugmentedMatrix>& queue,
                   size_t n_min, size_t n_max)
    : q(queue), n_min(n_min), n_max(n_max)
{
    active_producers.fetch_add(1, std::memory_order_relaxed);
}

void Producer::operator()()
{
    const std::string tasks_dir = "data/Задачи";

    for (size_t i = n_min; i <= n_max; ++i) {
        const std::string filename   = std::to_string(i) + ".txt";
        const std::string file_path  = tasks_dir + "/" + filename;
        std::ifstream in(file_path);
        if (!in) {
            std::cerr << "Ошибка: не удалось открыть " << file_path << '\n';
            continue;
        }

        size_t n;
        if (!(in >> n) || n == 0) {
            std::cerr << "Ошибка формата: " << file_path << '\n';
            continue;
        }

        AugmentedMatrix task;
        task.A = std::make_unique< Matrix<double> >(n, n);
        task.B = std::make_unique< Matrix<double> >(n, 1);
        task.X = std::make_unique< Matrix<double> >(n, 1);
        task.Filename = filename;

        // ───── чтение главной диагонали ─────
        for (size_t j = 0; j < n; ++j) {
            if (!(in >> (*task.A)(j, j))) { goto read_error; }
        }
        // ───── чтение верхней диагонали ─────
        for (size_t j = 0; j + 1 < n; ++j) {
            if (!(in >> (*task.A)(j, j + 1))) { goto read_error; }
        }
        // ───── чтение правой части ─────
        for (size_t j = 0; j < n; ++j) {
            if (!(in >> (*task.B)(j, 0))) { goto read_error; }
        }
        in.close();
        q.push(std::move(task));
        continue;                           // ok

read_error:
        std::cerr << "Ошибка чтения данных в " << file_path << '\n';
    }

    // ── Конец работы Producer-а: отправляем сентинелы ──
    if (active_producers.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        for (int k = 0; k < total_consumers; ++k) {
            q.push(AugmentedMatrix{});      // пустой = сигнал завершения
        }
    }
}

// -------------------------------------------------------
//  Consumer
// -------------------------------------------------------
Consumer::Consumer(Bounded<AugmentedMatrix>& queue, size_t /*unused*/)
    : q(queue)
{
    total_consumers.fetch_add(1, std::memory_order_relaxed);
}

void Consumer::operator()()
{
    namespace fs = std::filesystem;
    fs::create_directories("data/Решения");

    while (true) {
        AugmentedMatrix task;
        q.pop(task);

        if (task.Filename.empty()) break;          // пришёл сентинел

        const size_t n = task.A->m_rows();

        // ---------- решение в float ----------
        Matrix<float>  A_f(n, n), B_f(n, 1), X_f(n, 1);

        for (size_t i = 0; i < n; ++i) {
            B_f(i,0) = static_cast<float>((*task.B)(i,0));
            for (size_t j = 0; j < n; ++j) A_f(i,j) = 0.0f;
            A_f(i,i) = static_cast<float>((*task.A)(i,i));
            if (i + 1 < n)
                A_f(i,i+1) = static_cast<float>((*task.A)(i,i+1));
        }
        try { solve(A_f, X_f, B_f); }
        catch (const std::exception& e) {
            std::cerr << "float-решатель: " << e.what()
                      << "  (" << task.Filename << ")\n";
        }

        // ---------- решение в double ----------
        try { solve(*task.A, *task.X, *task.B); }
        catch (const std::exception& e) {
            std::cerr << "double-решатель: " << e.what()
                      << "  (" << task.Filename << ")\n";
        }

        // ---------- относительные погрешности ----------
        auto rel_err = [&](auto& A, auto& X, auto& B) {
            using T = std::decay_t<decltype(A(0,0))>;
            T r2 = 0, b2 = 0;
            for (size_t i = 0; i < n; ++i) {
                T Ax = A(i,i) * X(i,0) +
                       (i + 1 < n ? A(i,i+1)*X(i+1,0) : T{});
                T r  = Ax - B(i,0);
                r2  += r*r;
                b2  += B(i,0)*B(i,0);
            }
            return std::sqrt(r2) /
                   (std::sqrt(b2) + std::numeric_limits<T>::epsilon());
        };
        const float  rel_f = rel_err(A_f, *reinterpret_cast<Matrix<float>*>(&X_f), B_f);
        const double rel_d = rel_err(*task.A, *task.X, *task.B);

        // ---------- запись решений ----------
        auto stem = task.Filename.substr(0, task.Filename.size() - 4); // «.txt»
        std::ofstream("data/Решения/"+stem+"_float.txt")
            << std::scientific << std::setprecision(8)
            << X_f;                              // оператор << у Matrix

        std::ofstream("data/Решения/" + stem + "_double.txt")
            << std::scientific << std::setprecision(16)
            << *task.X;

        // ---------- вывод на консоль ----------
        std::cout << std::scientific << std::setprecision(8)
                  << "[float ] " << task.Filename
                  << "  rel.err = " << rel_f << '\n'
                  << "[double] " << task.Filename
                  << "  rel.err = " << rel_d << '\n';
    }
}
