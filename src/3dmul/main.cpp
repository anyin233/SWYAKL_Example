#include "YAKL.h"
#include "YAKL_finalize.h"
#include "YAKL_timers.h"

const int N = 1 << 8;
const int M = 1 << 8;
const int Q = 1 << 8;

using yakl::Array;
using yakl::memDevice;
using yakl::memHost;
using yakl::styleC;
using yakl::c::Bounds;
using yakl::c::parallel_for;

using C3ArrayH = yakl::Array<int, 3, yakl::memHost, yakl::styleC>;
using C3Array = yakl::Array<int, 3, yakl::memDevice, yakl::styleC>;

int main() {
  std::cout << "=========================================" << std::endl;
  std::cout << "    Running 3D Multiplication Example    " << std::endl;
  std::cout << "=========================================" << std::endl;

  yakl::init();
  {
    C3Array a("a", N, M, Q), b("b", N, M, Q), c("c", N, M, Q);
    C3ArrayH ah("ah", N, M, Q), bh("bh", N, M, Q), ch("ch", N, M, Q);

    yakl::c::parallel_for(
        "Initialize", yakl::c::Bounds<3>(N, M, Q),
        YAKL_LAMBDA(int i, int j, int k) {
          a(i, j, k) = i + j + k;
          b(i, j, k) = i + j + k + 1;
          c(i, j, k) = 0;
        });

    for (int i = 0; i < N; i++) {
      for (int j = 0; j < M; j++) {
        for (int k = 0; k < Q; k++) {
          ah(i, j, k) = i + j + k;
          bh(i, j, k) = i + j + k + 1;
        }
      }
    }

    std::cout << "Initialize finished" << std::endl;

    yakl::timer_start("Serial 3D Multiplication");
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < M; j++) {
        for (int k = 0; k < Q; k++) {
          ch(i, j, k) = ah(i, j, k) * bh(i, j, k);
        }
      }
    }
    yakl::timer_stop("Serial 3D Multiplication");

    yakl::timer_start("Legacy 3D Kernel");
    yakl::c::parallel_for(
        "3D Multiplication", yakl::c::Bounds<1>(N * M * Q),
        YAKL_LAMBDA(int i) {
          int kk = i % Q;
          int jj = (i / Q) % M;
          int ii = i / (Q * M);
          ch(ii, jj, kk) = ah(ii, jj, kk) * bh(ii, jj, kk);
        });
    yakl::timer_stop("Legacy 3D Kernel");

    yakl::timer_start("3D Kernel");
    yakl::c::parallel_for(
        "3D Multiplication", yakl::c::Bounds<3>(N, M, Q),
        YAKL_LAMBDA(int i, int j, int k) {
          c(i, j, k) = a(i, j, k) * b(i, j, k);
        });
    yakl::timer_stop("3D Kernel");
  }
  yakl::finalize();
}