#include <stdio.h>

void loop_example(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2;
    }
}

int main() {
    int arr[5] = {1, 2, 3, 4, 5};
    loop_example(arr, 5);
    return 0;
}

