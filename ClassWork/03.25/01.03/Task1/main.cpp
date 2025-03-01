#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>

using size = std::size_t;

class Matrix {
protected:
    size m;
    size n;
public:
    Matrix(size rows, size cols) : m(rows), n(cols) {}
    virtual double & at(size row, size col) = 0;
    virtual double & operator()(size row, size col) = 0;
    size numRows() const { return m; }
    size numCols() const { return n; }
    virtual ~Matrix() {}
    virtual void print() = 0;
};

class Matrix_Diagonal : public Matrix {
private:
    size L;
    size U;
    std::vector<std::vector<double>> diagonals;
    
public:
    Matrix_Diagonal(size rows, size cols, size lower, size upper)
        : Matrix(rows, cols), L(lower), U(upper)
    {
        diagonals.resize(lower + upper + 1);
        for (size k = 0; k < diagonals.size(); ++k) {
            int d = static_cast<int>(k) - static_cast<int>(lower);
            size diag_size = (d >= 0) ? std::min(m, n - static_cast<size>(d))
                                      : std::min(n, m - static_cast<size>(-d));
            diagonals[k].resize(diag_size, 0.0);
        }
    }
    
    class RowProxy {
    private:
        Matrix_Diagonal & matrix;
        size row;
    public:
        RowProxy(Matrix_Diagonal & mat, size r) : matrix(mat), row(r) {}
        double & operator[](size col) { return matrix.at(row, col); }
    };
    
    double & at(size i, size j) override {
        static double zero = 0.0;
        if (i >= m || j >= n)
            throw std::out_of_range("Индекс вне диапазона");
        int d = static_cast<int>(j) - static_cast<int>(i);
        if (d < -static_cast<int>(L) || d > static_cast<int>(U))
            return zero;
        size diag_index = static_cast<size>(d + static_cast<int>(L));
        size pos = (d >= 0) ? i : j;
        if (pos >= diagonals[diag_index].size())
            return zero;
        return diagonals[diag_index][pos];
    }
    
    double & operator()(size i, size j) override {
        return at(i, j);
    }
    
    ~Matrix_Diagonal() override {
        for (auto &diag : diagonals)
            diag.clear();
        diagonals.clear();
    }
    
    RowProxy operator[](size row) { return RowProxy(*this, row); }
    
    void print() override {
        for (size i = 0; i < m; ++i) {
            for (size j = 0; j < n; ++j)
                std::cout << at(i, j) << "\t";
            std::cout << "\n";
        }
    }
};

class Matrix_Square_2D : public Matrix_Diagonal {
public:
    Matrix_Square_2D(size n) : Matrix_Diagonal(n, n, 0, 1) {
        for (size i = 0; i < n; ++i) {
            at(i, i) = 7.0 / 5.0;
        }
        for (size i = 0; i < n - 1; ++i) {
            at(i, i + 1) = 11.0 / 3.0;
        }
    }
};

class Matrix_Array : public Matrix {
private:
    std::vector<double> data;
    
public:
    Matrix_Array(size rows, size cols) : Matrix(rows, cols), data(rows * cols, 0.0) {
        for (size k = 1; k < rows; ++k) {
            double numerator = 152 * k + 118;
            double denominator = 15 * (2 * k + 1) * (2 * k + 3);
            data[k] = numerator / denominator;
        }
        data[rows - 1] = 7.0 / (5 * (2 * (rows - 1) + 1));
    }
    
    double & at(size row, size col) override {
        if (row >= m || col >= n)
            throw std::out_of_range("Индекс вне диапазона");
        return data[row * n + col];
    }
    
    double & operator()(size row, size col) override {
        return at(row, col);
    }
    
    void print() override {
        for (size i = 0; i < m; ++i) {
            std::cout << at(i, 0) << "\n";
        }
    }
};

int main() {
    size N = 7;

    // Matrix_Square_2D
    Matrix_Square_2D A(N);
    std::cout << "Matrix_Square_2D:\n";
    A.print();
    
    std::cout << "\n";

    // Matrix_Array
    Matrix_Array B(N, 1);
    std::cout << "Matrix_Array:\n";
    B.print();

    return 0;
}
