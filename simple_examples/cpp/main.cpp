#include <iostream>

constexpr int N = 1 << 10;
constexpr int M = 1 << 10;

int main (){
    int a[N], b[N], c[N];

    for (int i = 0; i < N; i ++) {
        a[i] = i;
        b[i] = i + 1;
    }

    for (int i = 0; i < N; i ++) {
        c[i] = a[i] + b[i];
    }
}