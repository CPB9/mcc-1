#include <iostream>

#define ITERATION 64

template < int i >
class  A
{
    A<i-1>   x;
    A<i-2>   y;
};
template <> class A<0>
{
    char a;
};
template <> class A<1>
{
    char a;
};

int main(void)
{
    A<ITERATION> b;    // YOU MIGHT WANT TO START WITH A MUCH SMALLER NUMBER HERE!
    std::cout<< "Itaration value is " << ITERATION << std::endl;
    return 0;
}
