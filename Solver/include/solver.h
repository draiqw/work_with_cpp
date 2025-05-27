#pragma once
#include <memory>
#include <atomic>
#include <string>
#include "matrix.h"
#include "bounded.h"

// ------------------------------------------------------------------
// Структура задачи: матрица A, вектор правой части B и вектор решения X
// ------------------------------------------------------------------
struct AugmentedMatrix {
    std::unique_ptr< Matrix<double> > A;
    std::unique_ptr< Matrix<double> > B;
    std::unique_ptr< Matrix<double> > X;
    std::string Filename;

    AugmentedMatrix();                                 // по-умолчанию (сентинел)
    AugmentedMatrix(AugmentedMatrix&&) noexcept;       // move-конструктор
    AugmentedMatrix& operator=(AugmentedMatrix&&) noexcept;

    AugmentedMatrix(const AugmentedMatrix&)            = delete;
    AugmentedMatrix& operator=(const AugmentedMatrix&) = delete;
};

// ------------------------------------------------------------------
// Producer: читает файлы из диапазона [n_min … n_max] и кладёт в очередь
// ------------------------------------------------------------------
struct Producer {
    Bounded<AugmentedMatrix>& q;
    size_t n_min, n_max;

    static std::atomic<int> active_producers;
    static int              total_consumers;          // задаётся до запуска

    Producer(Bounded<AugmentedMatrix>& queue,
             size_t n_min,
             size_t n_max);

    void operator()();                                // работа Producer-а
};

// ------------------------------------------------------------------
// Consumer: забирает задачи, решает их и пишет результаты
// ------------------------------------------------------------------
struct Consumer {
    Bounded<AugmentedMatrix>& q;

    static std::atomic<int> total_consumers;          // счётчик для сентинелов

    Consumer(Bounded<AugmentedMatrix>& queue, size_t /*unused*/);
    void operator()();
};
