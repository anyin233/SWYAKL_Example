

#include "YAKL.h"
#include <iostream>

typedef yakl::Array<double, 1, yakl::memDevice, yakl::styleC> real1d_device;
typedef yakl::Array<double, 1, yakl::memHost, yakl::styleC> real1d_host;
typedef yakl::Array<double, 2, yakl::memDevice, yakl::styleC> real2d_device;
typedef yakl::Array<double, 2, yakl::memHost, yakl::styleC> real2d_host;
typedef yakl::Array<double, 3, yakl::memDevice, yakl::styleC> real3d_device;
typedef yakl::Array<double, 3, yakl::memHost, yakl::styleC> real3d_host;

using yakl::Array;
using yakl::memDevice;
using yakl::memHost;
using yakl::styleC;
using yakl::c::Bounds;
using yakl::c::parallel_for;

using yakl::SArray;
using real = double;

int main(int argc, char **argv) {
  int N = 0;
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <N>" << std::endl;
    exit(1);
  }
  N = atoi(argv[1]);
  yakl::init();
  {
    real1d_device a("a", N), b("b", N), c("c", N);
    real1d_host ah("ah", N), bh("bh", N), ch("ch", N);
    yakl::c::parallel_for(
        "Initialize 1D", yakl::c::Bounds<1>(N), YAKL_LAMBDA(int i) {
          ah(i) = i;
          bh(i) = i + 1;
          ch(i) = 0;
        });

    printf("Expect: Pass\n");

    ah.deep_copy_to(a);
    bh.deep_copy_to(b);
    ch.deep_copy_to(bh);

    // printf("Expect: Error\n");

    // parallel_for("deep copy", Bounds<1>(N), YAKL_LAMBDA(int i) {
    //   a.deep_copy_to(ah);
    //   ch.deep_copy_to(c);
    // });
  }
}