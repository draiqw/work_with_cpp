#include <iostream>
#include <cmath>
#include <random>
using namespace std;



class Quaternion{
    double a, b, c, d;
public:
    Quaternion(double a, double b, double c, double d){
        this->a = a;
        this->b = b;
        this->c = c;
        this->d = d;
    }
    inline double get_a() const {return a;} //функция не меняет поля объекта => const
    inline double get_b() const {return b;}
    inline double get_c() const {return c;}
    inline double get_d() const {return d;}

    Quaternion(){a = 0; b = 0; c = 0; d = 0;}

    // можно сделать friend чтобы не использовать get_a() и т.д.
    // friend Quaternion operator*(const Quaternion &q1, const Quaternion &q2){
    //     double a_s = q1.a * q2.a - q1.b * q2.b - q1.c * q2.c - q1.d * q2.d;
    //     double b_s = q1.a * q2.b + q1.b * q2.a + q1.c * q2.d - q1.d * q2.c;
    //     double c_s = q1.a * q2.c - q1.b * q2.d + q1.c * q2.a + q1.d * q2.b;
    //     double d_s = q1.a * q2.d + q1.b * q2.c - q1.c * q2.b + q1.d * q2.a;
    //     return Quaternion(a_s, b_s, c_s, d_s);
    // }

    Quaternion conjugate() const{
        return Quaternion(a, -b, -c, -d);
    }
    inline double abs() const{
        return sqrt(a*a + b*b + c*c + d*d);
    }
    Quaternion inverse() const{
        double abs_q = abs()*abs();
        return Quaternion(a/abs_q, -b/abs_q, -c/abs_q, -d/abs_q);
    }
    Quaternion fillRandom(){
        a = rand() % 3 - 1;
        b = rand() % 3 - 1; 
        c = rand() % 3 - 1;
        d = rand() % 3 - 1;
        return *this;
    }
};

ostream & operator<<(ostream& os, Quaternion &q){
    return os << "{" << q.get_a() <<  ", " << q.get_b() << ", " << q.get_c() << ", " << q.get_d() << "}";
}

Quaternion operator+(const Quaternion &q1, const Quaternion &q2){
    double a_s = q1.get_a() + q2.get_a();
    double b_s = q1.get_b() + q2.get_b();
    double c_s = q1.get_c() + q2.get_c();
    double d_s = q1.get_d() + q2.get_d();
    return Quaternion(a_s, b_s, c_s, d_s);
}
Quaternion operator-(const Quaternion &q1, const Quaternion &q2){
    double a_s = q1.get_a() - q2.get_a();
    double b_s = q1.get_b() - q2.get_b();
    double c_s = q1.get_c() - q2.get_c();
    double d_s = q1.get_d() - q2.get_d();
    return Quaternion(a_s, b_s, c_s, d_s);
}
Quaternion operator*(const Quaternion &q1, const Quaternion &q2){
    double a_s = q1.get_a() * q2.get_a() - q1.get_b() * q2.get_b() - q1.get_c() * q2.get_c() - q1.get_d() * q2.get_d();
    double b_s = q1.get_a() * q2.get_b() + q1.get_b() * q2.get_a() + q1.get_c() * q2.get_d() - q1.get_d() * q2.get_c();
    double c_s = q1.get_a() * q2.get_c() - q1.get_b() * q2.get_d() + q1.get_c() * q2.get_a() + q1.get_d() * q2.get_b();
    double d_s = q1.get_a() * q2.get_d() + q1.get_b() * q2.get_c() - q1.get_c() * q2.get_b() + q1.get_d() * q2.get_a();
    return Quaternion(a_s, b_s, c_s, d_s);
}
Quaternion operator/(const Quaternion &q1, const Quaternion &q2){
    return q1 * q2.inverse();
}

class Vector3D{
    double x, y, z;
public:
    Vector3D(double x, double y, double z){
        this->x = x;
        this->y = y;
        this->z = z;
    }

    inline double get_x() const {return x;}
    inline double get_y() const {return y;}
    inline double get_z() const {return z;}

    Vector3D(){x = 0; y = 0; z = 0;}

    void rotate(Vector3D p, double phi);
};

void Vector3D::rotate(Vector3D p, double phi){
    double c = cos(phi/2);
    double s = sin(phi/2);
    Quaternion q(c, p.x*s, p.y*s, p.z*s);
    Quaternion w(0, x, y, z);
    Quaternion res = q * w * q.conjugate();
    x = res.get_b();
    y = res.get_c();
    z = res.get_d();
}

ostream & operator<<(ostream& os, Vector3D &v){
    return os << "{" << v.get_x() <<  ", " << v.get_y() << ", " << v.get_z() << "}";
}

class MatrixNxN{
// private:
    int n;
    Quaternion *matrix = new Quaternion[n*n];
public:
    MatrixNxN(int n){
        this->n = n;
        matrix = new Quaternion[n*n];
        for (int i = 0; i < n*n; i++){
            matrix[i].fillRandom();
        }
    }

    ~MatrixNxN(){
        delete[] matrix;
    }

    Quaternion get(int i, int j){
        return matrix[i*n + j];
    }

    Quaternion determinate(MatrixNxN &v);

    friend ostream & operator<<(ostream& os, MatrixNxN &v);

    Quaternion determinant3x3(MatrixNxN &v);
};

Quaternion MatrixNxN::determinate(MatrixNxN &v){
    Quaternion det(1, 0, 0, 0);
    Quaternion zero(0, 0, 0, 0);

    for (int i = 0; i < v.n; i ++){
        int privot = i;

        for (int j = i + 1; j < v.n; j ++){
            if (v.get(j, i).abs() > v.get(privot, i).abs()){
                privot = j;
            }
        }

        if (v.get(privot, i).abs() == 0){
            return det;
        }

        if (privot != i){
            swap(v.matrix[i], v.matrix[privot]);
            det = zero-det;
        }

        det = det * v.get(i, i);
        Quaternion inv = v.get(i, i).inverse();

        for (int j = i + 1; j < v.n; j ++){
            Quaternion factor = v.get(j, i) * inv;

            for (int k = i; k < v.n; k ++){
                v.matrix[j*v.n + k] = v.matrix[j*v.n + k] - factor * v.matrix[i*v.n + k];
            }
        }
    }
    return det;
}
ostream & operator<<(ostream& os, MatrixNxN &v){
    for (int i = 0; i < v.n; i++){
        for (int j = 0; j < v.n; j++){
            os << v.matrix[i*v.n + j] << " | ";
        }
        os << endl;
    }
    return os;
}

Quaternion MatrixNxN::determinant3x3(MatrixNxN &v) {
    return v.get(0, 0) * (v.get(1, 1) * v.get(2, 2) - v.get(1, 2) * v.get(2, 1))
         - v.get(0, 1) * (v.get(1, 0) * v.get(2, 2) - v.get(1, 2) * v.get(2, 0))
         + v.get(0, 2) * (v.get(1, 0) * v.get(2, 1) - v.get(1, 1) * v.get(2, 0));
}

int main(){
    // Проверка вывода
    // cout << "Hello, World!" << endl;
    Quaternion q1(5, 6, 7, 8), q2(1, 2, 3, 4);

    // Проверка сложения, вычитания, умножения, деления квартернионов
    // Quaternion sum, mul, sub, del;
    // sum = q1 + q2;
    // sub = q1 - q2;
    // mul = q1 * q2;
    // del = q1 / q2;
    // cout << "q1 = " << q1 << endl;
    // cout << "q2 = " << q2 << endl;
    // cout << "sum = " << sum << endl;
    // cout << "sub = " << sub << endl;
    // cout << "mul = " << mul << endl;
    // cout << "del = " << del << endl;

    // Проверка reverse
    // Quaternion req1 = q1.inverse();
    // mul = q1 * req1;
    // cout << "req1 = " << mul << endl;

    // Проверка поворота вектора
    // Vector3D v1(1, 3, -5);
    // v1.rotate(Vector3D(-1/sqrt(3), 1/sqrt(3), -1/sqrt(3)), 3.14159265359/3);
    // cout << "v1 = " << v1 << endl;

    MatrixNxN m(3);
    cout << m << endl;

    Quaternion det = m.determinate(m);
    cout << "det = " << det << endl;

    Quaternion det3x3 = m.determinant3x3(m);
    cout << "det3x3 = " << det3x3 << endl;
    return 0;
}