// main.cpp
#include <YAKL.h>
#include <iostream>

constexpr int N = 1 << 20;

using yakl::Array;
using yakl::COLON;
using yakl::memDevice;
using yakl::memHost;
using yakl::styleC;
using yakl::c::Bounds;
using yakl::c::parallel_for;
using yakl::c::SimpleBounds;

typedef double real;

typedef Array<real, 1, memHost, styleC> realHost1d;

typedef Array<real, 1, memDevice, styleC> real1d;

extern "C" {
double add_numbers(double, double);
double slave_add_numbers(double, double);
}

int main() {
  double a = 3.5, b = 4.5;
  double result = add_numbers(a, b);
  std::cout << "The result of " << a << " + " << b << " is " << result
            << std::endl;
  yakl::init();
  {
    real1d aarr("arr", N), barr("barr", N), carr("carr", N);

    parallel_for(
        "add", N, YAKL_LAMBDA(int i) {
          aarr(i) = 1 + i;
          barr(i) = 2 + i;
          carr(i) = slave_add_numbers(aarr(i), barr(i));
        });
    
    for (int i = 0; i < 20; i ++) {
      std::cout << "carr(" << i << ") = " << carr(i) << " ";
    }
  }
  return 0;
}