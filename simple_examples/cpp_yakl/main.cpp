#include <iostream>
#include "YAKL.h"

constexpr int N = 1 << 10;
constexpr int M = 1 << 10;

using yakl::c::parallel_for;
using yakl::c::Bounds;
using yakl::Array;
using yakl::memHost;
using yakl::memDevice;
using yakl::styleC;


typedef Array<int, 1, memHost, styleC> arr1d_host;
typedef Array<int, 1, memDevice, styleC> arr1d;

int main (){
    arr1d_host c_host(N);
    arr1d a("a", N), b("b", N), c("c", N);

    parallel_for("Init", Bounds<1>(N), YAKL_LAMBDA (int i) {
        a(i) = i;
        b(i) = i + 1;
    });

    parallel_for("compute", N, YAKL_LAMBDA(int i) {
        c(i) = a(i) + b(i);
    });

    c.deep_copy_to(c_host);
}