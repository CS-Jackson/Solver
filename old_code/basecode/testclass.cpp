#include <iostream>
using namespace std;

class A
{
    int a;
    // char b;
public:
    virtual double foo(){cout << "A" << endl;}
    //char x;
};

class B:public A
{
public:
    virtual double foo(){ cout << "B" << endl;}
};

class C: public virtual A
{
public:
    virtual double foo(){ }
};

class D
{
    int b;
    char a;
};

class BB
{
    int m_bb;
public:
    virtual ~BB(){}
    virtual void vfbb() {}
};
class B1 :  public virtual BB
{
    int m_b1;
public:
    virtual ~B1() {}
    virtual void vfb1() {}
};
class B2 :  public virtual BB
{
    int m_b2;
public:
    virtual ~B2() {}
    virtual void vfb2() {}
};
class DD : public B1, public B2
{
    //int m_dd;
public:
    virtual ~DD() {}
    virtual void vfdd() {}
};

int main()
{
    A *a = new A();
    // cout << sizeof(a) << endl;
    cout << sizeof(*a) << endl;

    B *b = new B();
    cout << sizeof(*b) << endl;
    //b->foo();

    C c = C();
    cout << sizeof(c) << endl;

    D d = D();
    cout << sizeof(d) << endl;

    BB bb = BB();
    B1 b1 = B1();
    B2 b2 = B2();
    DD dd = DD();
    cout << "BB: " << sizeof(bb) << endl;
    cout << "B1: " <<sizeof(b1) << endl;
    cout << "B2: " <<sizeof(b2) << endl;
    cout << "DD: " <<sizeof(dd) << endl;
    return 0;
}