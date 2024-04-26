#include "YAKL.h"
#include <chrono>
#include <iomanip>
#include <ios>
#include <iostream>
#include <istream>

const size_t N = 1 << 10;
const size_t M = 1 << 10;
using C1ArrayH = yakl::Array<int, 1, yakl::memHost, yakl::styleC>;
using C2ArrayH = yakl::Array<int, 2, yakl::memHost, yakl::styleC>;
using C1Array = yakl::Array<int, 1, yakl::memDevice, yakl::styleC>;
using C2Array = yakl::Array<int, 2, yakl::memDevice, yakl::styleC>;

[[gnu::kernel]] void print_size() {
  if (_PEN == 0) {
    std::cout << "CPE sizeof yakl::swTimer " << sizeof(yakl::swTimer) << std::endl;
  }
}

// #pragma swuc push host
int main() {
  // std::ios_base::sync_with_stdio(false);
  // SW_BKPT(init);
  yakl::init();
  {
    for (size_t n = 6; n < 14; n++) {
      const size_t N = 1 << n;
      const size_t M = 1 << n;

      C1Array a("a", N), b("b", N), c("c", N);
      C1ArrayH ah("ah", N), bh("bh", N), ch("ch", N);

      yakl::c::parallel_for(
          "Initialize", yakl::c::Bounds<1>(N), YAKL_LAMBDA(int i) {
            a(i) = i;
            b(i) = i + 1;
            c(i) = 0;
          });

      for (int i = 0; i < N; i++) {
        ah(i) = i;
        bh(i) = i + 1;
      }
      std::cout << "Initialize finished\n";
        
      std::string format_name = "Serial 1D Kernel " + std::to_string(n);
      for (int i = 0; i < 10; i++) {
        format_name = "Serial 1D Kernel " + std::to_string(n);
        yakl::timer_start(format_name.c_str());
        for (int i = 0; i < N; i++) {
          ch(i) = ah(i) * bh(i);
        }
        yakl::timer_stop(format_name.c_str());

        format_name = "parallel_for 1D Kernel " + std::to_string(n);
        yakl::timer_start(format_name.c_str());
        yakl::c::parallel_for(
            "Compute", yakl::c::Bounds<1>(N),
            YAKL_LAMBDA(int i) { c(i) = a(i) * b(i); });
        yakl::timer_stop(format_name.c_str());
      }

      format_name = "Set 2D Array " + std::to_string(n);

      C2Array aa("aa", N, M), bb("bb", N, M), cc("cc", N, M);
      C2ArrayH aah("aah", N, M), bbh("bbh", N, M), cch("cch", N, M);
      yakl::c::parallel_outer(
          "Compute Hierarchical", yakl::c::Bounds<1>(N),
          YAKL_LAMBDA(int i, yakl::InnerHandler handler) {
            yakl::c::parallel_inner(
                N,
                [&](int j) {
                  aa(i, j) = i * N + j;
                  bb(i, j) = j * N + j;
                  cc(i, j) = 0;
                },
                handler);
          });


      for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
          aah(i, j) = i * N + j;
          bbh(i, j) = j * N + j;
        }
      }

      for (int i = 0; i < 10; i++) {
        format_name = "Serial 2D Kernel " + std::to_string(n);
        yakl::timer_start(format_name.c_str());
        for (int i = 0; i < N; i++) {
          for (int j = 0; j < M; j++) {
            cch(i, j) = aah(i, j) * bbh(i, j);
          }
        }
        yakl::timer_stop(format_name.c_str());

        format_name = "parallel_for 2D Kernel " + std::to_string(n);
        yakl::timer_start(format_name.c_str());
        yakl::c::parallel_for(
            "Compute 2D", yakl::c::Bounds<2>(N, M),
            YAKL_LAMBDA(int i, int j) { cc(i, j) = aa(i, j) * bb(i, j); });
        yakl::timer_stop(format_name.c_str());

        format_name = "Hierarchical Parallel Kernel " + std::to_string(n);
        yakl::timer_start(format_name.c_str());
        yakl::c::parallel_outer(
            "Compute Hierarchical", yakl::c::Bounds<1>(N),
            YAKL_LAMBDA(int i, yakl::InnerHandler handler) {
              yakl::c::parallel_inner(
                  N, [&](int j) { cc(i, j) = aa(i, j) * bb(i, j); }, handler);
            },
            yakl::LaunchConfig<1>());
        yakl::timer_stop(format_name.c_str());
      }

    }
  }

  yakl::finalize();
  std::cout << "Program Exiting" << std::endl;
  SW_BKPT(finalize);
}
