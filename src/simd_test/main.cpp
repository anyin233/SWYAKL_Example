#include "YAKL.h"
#include "YAKL_timers.h"
#include <iostream>
#include "swperf.h"

extern "C" {
  void penv_slave_fd_float_sum_init();
  void penv_slave_fd_float_count(unsigned long *slave_fd_count);
}

const int N = 1 << 22;
const int SIMD_LEN = 8;
unsigned long slave_fd_count[64];
unsigned long slave_fd_count_sum_before;
unsigned long slave_fd_count_sum_end;
unsigned long slave_fd_count_sum;

using yakl::Array;
using yakl::memDevice;
using yakl::memHost;
using yakl::styleC;
using yakl::c::Bounds;
using yakl::c::parallel_for;
using yakl::simd::Pack;

using C1ArrayH = yakl::Array<int, 1, yakl::memHost, yakl::styleC>;
using C2ArrayH = yakl::Array<int, 2, yakl::memHost, yakl::styleC>;
using C1Array = yakl::Array<int, 1, yakl::memDevice, yakl::styleC>;
using C2Array = yakl::Array<int, 2, yakl::memDevice, yakl::styleC>;

int main() {
  std::cout << "=========================================" << std::endl;
  std::cout << "           Running SIMD Example          " << std::endl;
  std::cout << "=========================================" << std::endl;
  yakl::init();
  {
  // penv_slave_fd_float_sum_init();
    C1Array a("a", N), b("b", N), c("c", N);
    C1ArrayH ah("ah", N), bh("bh", N), ch("ch", N);
    yakl::c::parallel_for(
        "Initialize 1D", yakl::c::Bounds<1>(N), YAKL_LAMBDA(int i) {
          ah(i) = i;
          bh(i) = i + 1;
          ch(i) = 0;
        });

    printf("Run Normal 1D\n");
    for (int t = 0; t < 100; t++) {
      yakl::timer_start("Normal 1D");
      yakl::c::parallel_for(
          "Normal 1D", yakl::c::Bounds<1>(N),
          YAKL_LAMBDA(int i) { ch(i) = ah(i) * bh(i); ch(i) += 1;});
      yakl::timer_stop("Normal 1D");

      int blk_size = N / SIMD_LEN;
      printf("Run SIMD @ %d\n", SIMD_LEN);

      // penv_slave_fd_float_count(&slave_fd_count_sum_before);
      yakl::timer_start("SIMD 1D");
      yakl::c::parallel_for(
          "SIMD 1D", yakl::c::Bounds<1>(blk_size), YAKL_LAMBDA(int i) {
            int simd_i = i * SIMD_LEN;
            int simd_end = simd_i + SIMD_LEN;
            Pack<double, SIMD_LEN> av, bv, cv;
            for (int ii = 0; ii < SIMD_LEN; ii++) {
              av(ii) = ah(simd_i + ii);
              bv(ii) = bh(simd_i + ii);
            }
            cv = av * bv;
            cv = cv + 1;
            for (int ii = 0; ii < SIMD_LEN; ii++) {
              ch(simd_i + ii) = cv(ii);
            }
          });
      yakl::timer_stop("SIMD 1D");
    }
  }
  yakl::finalize();
}