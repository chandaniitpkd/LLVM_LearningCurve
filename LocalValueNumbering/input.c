#include <stdio.h>

int compute(int a, int b) {
    int x = a + b; 
    int y = a + b; 
    int z = x * 2; 
    int w = y * 2; 

    return z + w;  
}

int main() {
    int result = compute(5, 10);
    printf("Result: %d\n", result);
    return 0;
}

