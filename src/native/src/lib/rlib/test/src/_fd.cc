#include <iostream>
#include <cstdio>
#include <unistd.h>

#define IOSTREAM_PRINT(str) std::cout << str "\n"
//#define STDIO_PRINT(str) std::printf(str "\n")
#define STDIO_PRINT(str)
#define UNISTD_PRINT(str) write(1,str "\n",sizeof(str)+1)

int main() {
    std::ios::sync_with_stdio(false);
    IOSTREAM_PRINT("1");
    STDIO_PRINT("2");
    UNISTD_PRINT("3");
    IOSTREAM_PRINT("4");
    STDIO_PRINT("5");
    UNISTD_PRINT("6");
    IOSTREAM_PRINT("7");
    STDIO_PRINT("8");
    UNISTD_PRINT("9");
    IOSTREAM_PRINT("10");
    STDIO_PRINT("11");
    UNISTD_PRINT("12");
}
