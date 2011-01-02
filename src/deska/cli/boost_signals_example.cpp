#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <iostream>

using namespace boost;
using namespace std;

class ClassA
{
public:

    signal< void () >    SignalA;
    signal< void ( int ) > SignalB;
};

class ClassB
{
public:

    void PrintFoo()
    {
        cout << "Foo" << endl;
    }
    
    void PrintBar( int i )
    {
        cout << "Bar: " << i << endl;
    }
};

int main()
{
    ClassA a;
    ClassB b1;
    ClassB b2;

    a.SignalA.connect( bind( &ClassB::PrintFoo, &b1 ) );
    a.SignalA.connect( bind( &ClassB::PrintBar, &b1, 10 ) );
    a.SignalB.connect( bind( &ClassB::PrintBar, &b1, _1 ) );
    a.SignalB.connect( bind( &ClassB::PrintBar, &b2, _1 ) );

    a.SignalA();
    a.SignalB(4);
}