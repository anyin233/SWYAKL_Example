#include "YAKL.h"
#include <chrono>
#include <iomanip>
#include <ios>
#include <iostream>
#include <istream>
// #include <swperf.h>

// extern "C" {
//   void penv_slave_fd_float_init();
//   void penv_slave_fd_float_sum_init();
//   void penv_slave_fd_float_count(unsigned long *count);
// }

const size_t N = 1 << 10;
const size_t M = 1 << 10;
using C1ArrayH = yakl::Array<double, 1, yakl::memHost, yakl::styleC>;
using C2ArrayH = yakl::Array<double, 2, yakl::memHost, yakl::styleC>;
using C1Array = yakl::Array<double, 1, yakl::memDevice, yakl::styleC>;
using C2Array = yakl::Array<double, 2, yakl::memDevice, yakl::styleC>;

// [[gnu::kernel]] void print_size() {
//   if (_PEN == 0) {
//     std::cout << "CPE sizeof yakl::swTimer " << sizeof(yakl::swTimer) <<
//     std::endl;
//   }
// }

// #pragma swuc push host
int main() {
  // std::ios_base::sync_with_stdio(false);
  // SW_BKPT(init);
  yakl::init();
  unsigned long slave_fd_float_cnt_start = 0, slave_fd_float_cnt_end = 0;
  {
    // penv_slave_fd_float_sum_init();
    // penv_slave_fd_float_count(&slave_fd_float_cnt_start);
    asm volatile("nop");
    // SW_BKPT(perf);
    for (size_t n = 6; n < 14; n++) {
      std::cout << "Allocating 2^" << n << std::endl;
      const int N = 1 << n;
      const int M = 1 << n;

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
            "Compute", yakl::c::Bounds<1>((N - 1) / 8 + 1), YAKL_LAMBDA(int i) {
              yakl::simd::Pack<double, 8> a_pack, b_pack;
              int i_start = i * 8;
              int i_end = std::min(i_start + 8, N);
              for (int i = i_start; i < i_end; i++) {
                a_pack(i - i_start) = a(i);
                b_pack(i - i_start) = b(i);
              }
              a_pack *= b_pack;
              for (int i = i_start; i < i_end; i++) {
                c(i) = a_pack(i - i_start);
              }
            });
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
            "Compute 2D", yakl::c::Bounds<2>(N, (M - 1) / 8 + 1),
            YAKL_LAMBDA(int i, int j) {
              int j_start = j * 8;
              int j_end = std::min(j_start + 8, M);
              yakl::simd::Pack<double, 8> a_pack, b_pack;
              for (int j = j_start; j < j_end; j++) {
                a_pack(j - j_start) = aa(i, j);
                b_pack(j - j_start) = bb(i, j);
              }
              a_pack *= b_pack;
              for (int j = j_start; j < j_end; j++) {
                cc(i, j) = a_pack(j - j_start);
              }
            });
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
      if (n == 6) {
        for (int i = 0; i < N; i++) {
          for (int j = 0; j < N; j++) {
            if (cch(i, j) != cc(i, j)) {
              std::cout << "Error at " << i << " " << j << " c = " << cch(i, j)
                        << " " << cc(i, j) << std::endl;
              std::cout << "Error at " << i << " " << j << " a = " << aah(i, j)
                        << " " << aa(i, j) << std::endl;
              std::cout << "Error at " << i << " " << j << " b = " << bbh(i, j)
                        << " " << bb(i, j) << std::endl;
              yakl::finalize();
              return 1;
            }
          }
        }
      }
    }
    // penv_slave_fd_float_count(&slave_fd_float_cnt_end);
    printf("Total Float inst count: %lu\n",
           slave_fd_float_cnt_end - slave_fd_float_cnt_start);
  }

  yakl::finalize();
  std::cout << "Program Exiting" << std::endl;
}
