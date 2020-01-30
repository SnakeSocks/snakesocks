#define a b
#include <rlib/macro.hpp>
#include <rlib/print.hpp>

#define ma 'c'
#define mb a
using rlib::println;

#define b 'c'
int main()
{
    if(MACRO_EQL(ma, mb))
    {
        println("Hello world.");
    }
    println(MACRO_TO_CSTR(a));
    println("done");

    return 0;

}
