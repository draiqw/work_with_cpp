#include <iostream>

using namespace std;

class Matrix {
private:
    int m;     // количество строк
    int n;     // количество столбцов
    int** data;      // указатель на данные матрицы

    // Прокси-класс для строки матрицы
    class RowProxy {
    private:
        int* row;  // Указатель на строку
        int cols;  // Количество столбцов

    public:
        RowProxy(int* row, int cols) : row(row), cols(cols) {}

        // Перегрузка оператора [] для доступа к элементам строки
        int& operator[](int j) {
            if (j >= cols) {
                std::cerr << "Error: Column index out of range!" << std::endl;
                return row[0];  // Возвращаем первый элемент
            }
            return row[j];
        }
    };

public:
    // Конструктор по умолчанию (создает пустую матрицу)
    Matrix() : m(0), n(0), data(nullptr) {}

    // Основной конструктор (создает матрицу размера m x n)
    Matrix(int m, int n) : m(m), n(n) {
        data = new int*[m];
        for (int i = 0; i < m; ++i) {
            data[i] = new int[n]();  // инициализация нулями
        }
    }

    // Деструктор (освобождает память)
    ~Matrix() {
        if (data) {
            for (int i = 0; i < m; ++i) {
                delete[] data[i];
            }
            delete[] data;
        }
    }

    // Конструктор копирования  
    Matrix(const Matrix& other) : m(other.m), n(other.n) {
        data = new int*[m];
        for (int i = 0; i < m; ++i) {
            data[i] = new int[n];
            for (int j = 0; j < n; ++j) {
                data[i][j] = other.data[i][j];
            }
        }
    }

    // Оператор присваивания копированием
    Matrix& operator=(const Matrix& other) {
        if (this != &other) {  
            if (data) {
                for (int i = 0; i < m; ++i) {
                    delete[] data[i];
                }
                delete[] data;
            }

            m = other.m;
            n = other.n;

            data = new int*[m];
            for (int i = 0; i < m; ++i) {
                data[i] = new int[n];
                for (int j = 0; j < n; ++j) {
                    data[i][j] = other.data[i][j];
                }
            }
        }
        return *this;
    }

    // Конструктор перемещения
    Matrix(Matrix&& other)
        : m(other.m), n(other.n), data(other.data) {
        // Обнуляем указатели у исходного объекта
        other.m = 0;
        other.n = 0;
        other.data = nullptr;
    }

    // Оператор присваивания перемещением
    Matrix& operator=(Matrix&& other) {
        if (this != &other) {  // Проверка на самоприсваивание
            // Освобождаем старую память
            if (data) {
                for (int i = 0; i < m; ++i) {
                    delete[] data[i];
                }
                delete[] data;
            }

            // Перемещаем данные
            m = other.m;
            n = other.n;
            data = other.data;

            // Обнуляем указатели у исходного объекта
            other.m = 0;
            other.n = 0;
            other.data = nullptr;
        }
        return *this;
    }

    // Оператор доступа к элементам матрицы (без проверки границ)
    int& operator()(int i, int j) {
        return data[i][j];  // Нет проверки границ!
    }

    // Метод для вывода матрицы на экран
    void print() const {
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                cout << data[i][j] << " ";
            }
            cout << endl;
        }
    }

    // Статический метод для создания единичной матрицы
    static Matrix eye(int n) {
        Matrix result(n, n);  // Создаем квадратную матрицу n x n
        for (int i = 0; i < n; ++i) {
            result(i, i) = 1;  // Заполняем диагональ единицами
        }
        return result;
    }

    
};

int main() {
    // Создаем матрицу 3x3
    Matrix mat(3, 3);


    // Заполняем матрицу значениями
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            mat(i, j) = i * 3 + j + 1;  // Используем оператор () для доступа к элементам
        }
    }

    cout << "Original matrix:" << endl;
    mat.print();

    // Используем конструктор копирования
    Matrix mat2 = mat;
    cout << "Copied matrix:" << endl;
    mat2.print();

    // Используем конструктор перемещения
    Matrix mat3 = move(mat);
    cout << "Moved matrix:" << endl;
    mat3.print();

    // Проверяем, что исходная матрица теперь пуста
    cout << "Original matrix after move:" << endl;
    mat.print();

    Matrix identityMatrix = Matrix::eye(4);

    // Выводим единичную матрицу на экран
    cout << "Identity matrix (4x4):" << endl;
    identityMatrix.print();

    return 0;
}
