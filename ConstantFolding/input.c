#include <stdio.h>

int main() {
    volatile int a = 10;  // `volatile` prevents Clang from folding at compile time
    volatile int b = 20;
    volatile int c = 5;
    volatile int d = 6;

    int x = a + b;  // Now this addition will appear in IR
    int y = c * d;  // Multiplication also stays in IR
    int z = x + y;
    
    printf("%d\n", z);
    return 0;
}

