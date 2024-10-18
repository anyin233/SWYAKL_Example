#include "YAKL.h"
#include "YAKL_Bounds_c.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <random>

using real = double;

typedef yakl::Array<real, 1, yakl::memDevice, yakl::styleC> real1d_device;
typedef yakl::Array<real, 1, yakl::memHost, yakl::styleC> real1d_host;
typedef yakl::Array<real, 2, yakl::memDevice, yakl::styleC> real2d_device;
typedef yakl::Array<real, 2, yakl::memHost, yakl::styleC> real2d_host;
typedef yakl::Array<real, 3, yakl::memDevice, yakl::styleC> real3d_device;
typedef yakl::Array<real, 3, yakl::memHost, yakl::styleC> real3d_host;

using yakl::Array;
using yakl::memDevice;
using yakl::memHost;
using yakl::styleC;
using yakl::c::Bounds;
using yakl::c::parallel_for;

using yakl::SArray;

template <int N> inline int cnt(const int x) { return x; }

template <int N> inline int cnt2(const int x) {
  return std::abs(x * (x - cnt<N>(x)) / 1024);
}

template <int N> inline int cnt3(const int x) {
  return std::abs(rand() % (N * 2) - N);
}

int main() {
  yakl::init();
  srand(10);
  for (int times = 0; times < 10; times++) {

    const int N = 2048;
    real2d_device a_device("a_device", N + 2, N + 2),
        b_device("b_device", N + 2, N + 2);
    real2d_host a_host("a_host", N + 2, N + 2), b_host("b_host", N + 2, N + 2);
    real1d_host cnt_host("cnt_host", N * N);
    real1d_device cnt_device("cnt_device", N * N);
    for (int i = 1; i <= N; i++) {
      for (int j = 1; j <= N; j++) {
        a_host(i, j) = (double)rand();
        b_host(i, j) = 1;
        cnt_host(i + j) = cnt3<N>(i + j);
      }
    }

    std::mt19937 gen(10);
    std::normal_distribution d{static_cast<double>(N), 4.0};

    for (int i = 1; i <= N; i++) {
      for (int j = 1; j <= N; j++) {
        // cnt_host(i + j) = N - std::abs(N - std::sin(d(gen)));
        cnt_host(i + j) = cnt<N>(i + j);
      }
    }
    cnt_host.deep_copy_to(cnt_device);
    std::cout << "First 10 cnt";
    for (int i = 2; i < 12; i++) {
      std::cout << " " << cnt_host(i);
    }
    puts("");

    std::cout << "Starting unbalanced host" << std::endl;
    yakl::timer_start("unbalanced host");
    for (int i = 1; i <= N; i++) {
      for (int j = 1; j <= N; j++) {
        for (int k = 0; k < cnt_host(i + j); k++) {
          b_host(i, j) += a_host(i, j) + i + j + k;
        }
      }
    }
    yakl::timer_stop("unbalanced host");

    a_host.deep_copy_to(a_device);
    std::cout << "Starting unbalanced device" << std::endl;

    yakl::timer_start("unbalanced device");
    parallel_for(
        "unbalanced kernel", Bounds<2>({1, N + 1}, {1, N + 1}),
        YAKL_LAMBDA(int i, int j) {
          real b = 1;
          for (int k = 0; k < cnt_device(i + j); k++) {
            b += a_device(i, j) + i + j + k;
          }
          b_device(i, j) = b;
        });
    yakl::timer_stop("unbalanced device");

    yakl::timer_start("unbalanced device simd");
    parallel_for(
        "unbalanced kernel", Bounds<2>({1, N + 1}, {1, N + 1}),
        YAKL_LAMBDA(int i, int j) {
          real b = 1;
          int cur_cnt = cnt_device(i + j);
          if (cur_cnt >= 32) {
            yakl::simd::Pack<real, 8> a_packs;

            int num_packs = cur_cnt >> 3;
            int remains = cur_cnt & 7;
            for (int k = 0; k < num_packs; k++) {
              int start_index = k << 3;
              int end_index = start_index + 8;
              for (int kk = start_index; kk < end_index; kk++) {
                a_packs(kk - start_index) = a_device(i, j) + i + j + kk;
              }
              b += a_packs.sum();
            }
            int start_remain = num_packs << 3;
            for (int k = start_remain; k < start_remain + remains; k++) {
              b += a_device(i, j) + i + j + k;
            }
          } else {
            for (int k = 0; k < cur_cnt; k++) {
              b += a_device(i, j) + i + j + k;
            }
          }

          b_device(i, j) = b;
        });
    yakl::timer_stop("unbalanced device simd");
  }
  yakl::finalize();
}