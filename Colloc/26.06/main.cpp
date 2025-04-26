#include <iostream>
#include <list>    // добавляем <list> для std::list
#include <vector>
#include <cstdio>

class Base{
public:
    Base() { printf("1"); }
    virtual ~Base() { printf("2"); }
    void s() { printf("3"); }
    virtual void q() { printf("4"); }
    virtual void v() { printf("5"); s(); q(); }
};

class Derived: public Base{
public:
    Derived() { printf("6"); }
    ~Derived() { printf("7"); }
    void s() { printf("8"); }
    virtual void q() { printf("9"); }
    virtual void v() { printf("10"); Base::v(); }
};
void f(){
    Derived b; //16
    Base * pa = &b;
    pa -> s(); //3
    b.s(); //8
    pa -> v(); //10 5 3 9 
    // 7 2
}

int main() {
    f();
    return 0;
}