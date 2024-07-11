

#include "YAKL.h"
#include "YAKL_timers.h"
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

#define randq0 0.42902885331071894
#define randq1 0.3835149150254443
#define randq2 0.23069394385027253
#define randq3 0.8490940280251191
#define randq4 0.13880139717450857
#define randq5 0.7535669344593772
#define randq6 0.9871726780384124
#define randq7 0.6517359346213617
#define randq8 0.007027172843024565
#define randq9 0.18381318805149127
#define randq10 0.136064001401214
#define randq11 0.8714124615900947
#define randq12 0.8714348784956923
#define randq13 0.3958204533718198
#define randq14 0.2850986365562863
#define randq15 0.8170168349259794
#define randq16 0.847846769248984
#define randq17 0.6256118416818361
#define randq18 0.23735018356951787
#define randq19 0.5933040715916058
#define randq20 0.06763653209489273
#define randq21 0.4832035201020275
#define randq22 0.041225810690153075
#define randq23 0.030359155660743875
#define randq24 0.24026628330922173
#define randq25 0.8768136077897237
#define randq26 0.13218452098797873

int min(int a, int b) {
  if (a > b)
    return b;
  else
    return a;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s <N> <N_3D>\n", argv[0]);
    exit(1);
  }

  const int N = atoi(argv[1]);
  const int N_3D = atoi(argv[2]);

  yakl::init();
  {
    {
      printf("=========================================\n");
      printf("stencil-2d\n");
      printf("=========================================\n");
      // stencil-2d-5
      // real2d_host a_host2d("a_host", N + 2, N + 2), b_host2d("b_host", N + 2,
      // N + 2);
      real2d_device a_device2d("a_device2d", N + 2, N + 2),
          b_device2d("b_device2d", N + 2, N + 2);
      yakl::c::parallel_for(
          yakl::c::Bounds<2>({1, N + 1}, {1, N + 1}),
          YAKL_LAMBDA(int i, int j) {
            a_device2d(i, j) = (double)rand();
            b_device2d(i, j) = 0.0;
          });
      for (int i = 0; i < 20; i++) {
        yakl::timer_start("stencil-2d-5");
        yakl::c::parallel_for(
            yakl::c::Bounds<2>({1, N + 1}, {1, N + 1}),
            YAKL_LAMBDA(int i, int j) {
              yakl::SArray<double, 2, 3, 3> a;
              for (int ii = -1; ii <= 1; ii++)
                for (int jj = -1; jj <= 1; jj++)
                  a(ii + 1, jj + 1) = a_device2d(i + ii, j + jj);
              b_device2d(i, j) = randq0 * a(1, 1) + randq1 * a(0, 1) +
                                 randq2 * a(2, 1) + randq3 * a(1, 0) +
                                 randq4 * a(1, 2);
            });
        yakl::timer_stop("stencil-2d-5");
      }

      // stencil-2d-9
      // real2d_host a_host2d("a_host", N + 2, N + 2), b_host2d("b_host", N + 2,
      // N + 2); real2d_device a_device2d("a_device", N + 2, N + 2),
      // b_device2d("b_device", N + 2, N + 2);

      yakl::c::parallel_for(
          yakl::c::Bounds<2>({1, N + 1}, {1, N + 1}),
          YAKL_LAMBDA(int i, int j) {
            a_device2d(i, j) = (double)rand();
            b_device2d(i, j) = 0.0;
          });

      for (int i = 0; i < 20; i++) {
        yakl::timer_start("stencil-2d-9");
        yakl::c::parallel_for(
            yakl::c::Bounds<2>({1, N + 1}, {1, N + 1}),
            YAKL_LAMBDA(int i, int j) {
              yakl::SArray<double, 2, 3, 3> a;
              for (int ii = -1; ii <= 1; ii++)
                for (int jj = -1; jj <= 1; jj++)
                  a(ii + 1, jj + 1) = a_device2d(i + ii, j + jj);
              b_device2d(i, j) =
                  randq0 * a(1, 1) + randq1 * a(0, 1) + randq2 * a(2, 1) +
                  randq3 * a(1, 0) + randq4 * a(1, 2) + randq5 * a(0, 0) +
                  randq6 * a(0, 2) + randq7 * a(2, 0) + randq8 * a(2, 2);
            });
        yakl::timer_stop("stencil-2d-9");
      }
    }
    {
      printf("=========================================\n");
      printf("stencil-3d\n");
      printf("=========================================\n");
      // stencil-3d-7
      // real3d_host a_host3d("a_host3d", N + 2, N + 2, N + 2),
      // b_host3d("b_host3d", N + 2, N + 2, N + 2);
      const int N = N_3D;
      real3d_device a_device3d("a_device3d", N + 2, N + 2, N + 2),
          b_device3d("b_device3d", N + 2, N + 2, N + 2);

      yakl::c::parallel_for(
          yakl::c::Bounds<3>({1, N + 1}, {1, N + 1}, {1, N + 1}),
          YAKL_LAMBDA(int i, int j, int k) {
            a_device3d(i, j, k) = (double)rand();
            b_device3d(i, j, k) = 0.0;
          });
      for (int i = 0; i < 20; i++) {
        yakl::timer_start("stencil-3d-7");
        yakl::c::parallel_for(
            yakl::c::Bounds<3>({1, N + 1}, {1, N + 1}, {1, N + 1}),
            YAKL_LAMBDA(int i, int j, int k) {
              yakl::SArray<double, 3, 3, 3, 3> a;
              for (int ii = -1; ii <= 1; ii++)
                for (int jj = -1; jj <= 1; jj++)
                  for (int kk = -1; kk <= 1; kk++)
                    a(ii + 1, jj + 1, kk + 1) =
                        a_device3d(i + ii, j + jj, k + kk);
              b_device3d(i, j, k) = randq0 * a(1, 1, 1) + randq1 * a(0, 1, 1) +
                                    randq2 * a(2, 1, 1) + randq3 * a(1, 0, 1) +
                                    randq4 * a(1, 2, 1) + randq5 * a(1, 1, 0) +
                                    randq6 * a(1, 1, 2);
            });
        yakl::timer_stop("stencil-3d-7");
      }
      // stencil-3d-27
      // real3d_host a_host3d("a_host", N + 2, N + 2, N + 2), b_host3d("b_host",
      // N + 2, N + 2, N + 2); real3d_device a_device3d("a_device", N + 2, N +
      // 2, N + 2), b_device3d("b_device", N + 2, N + 2, N + 2);

      yakl::c::parallel_for(
          yakl::c::Bounds<3>({1, N + 1}, {1, N + 1}, {1, N + 1}),
          YAKL_LAMBDA(int i, int j, int k) {
            a_device3d(i, j, k) = (double)rand();
            b_device3d(i, j, k) = 0.0;
          });
      for (int i = 0; i < 20; i++) {
        yakl::timer_start("stencil-3d-27");
        yakl::c::parallel_for(
            yakl::c::Bounds<3>({1, N + 1}, {1, N + 1}, {1, N + 1}),
            YAKL_LAMBDA(int i, int j, int k) {
              yakl::SArray<double, 3, 3, 3, 3> a;
              for (int ii = -1; ii <= 1; ii++)
                for (int jj = -1; jj <= 1; jj++)
                  for (int kk = -1; kk <= 1; kk++)
                    a(ii + 1, jj + 1, kk + 1) =
                        a_device3d(i + ii, j + jj, k + kk);
              b_device3d(i, j, k) =
                  randq0 * a(1, 1, 1) + randq1 * a(0, 1, 1) +
                  randq2 * a(2, 1, 1) + randq3 * a(1, 0, 1) +
                  randq4 * a(1, 2, 1) + randq5 * a(1, 1, 0) +
                  randq6 * a(1, 1, 2) + randq7 * a(0, 0, 1) +
                  randq8 * a(0, 2, 1) + randq9 * a(2, 0, 1) +
                  randq10 * a(2, 2, 1) + randq11 * a(0, 1, 0) +
                  randq12 * a(0, 1, 2) + randq13 * a(2, 1, 0) +
                  randq14 * a(2, 1, 2) + randq15 * a(1, 0, 0) +
                  randq16 * a(1, 0, 2) + randq17 * a(1, 2, 0) +
                  randq18 * a(1, 2, 2) + randq19 * a(0, 0, 0) +
                  randq20 * a(0, 0, 2) + randq21 * a(0, 2, 0) +
                  randq22 * a(0, 2, 2) + randq23 * a(2, 0, 0) +
                  randq24 * a(2, 0, 2) + randq25 * a(2, 2, 0) +
                  randq26 * a(2, 2, 2);
            });
        yakl::timer_stop("stencil-3d-27");
      }
    }
    { // stencil-1d-3
      printf("=========================================\n");
      printf("stencil-1d\n");
      printf("=========================================\n");
      const int N_1D = N * N;
      real1d_device a_device1d("a_device1d", N_1D + 2),
          b_device1d("b_device1d", N_1D + 2);
      yakl::c::parallel_for(
          yakl::c::Bounds<1>(1, N_1D + 1), YAKL_LAMBDA(int i) {
            a_device1d(i) = (double)rand();
            b_device1d(i) = 0.0;
          });
      for (int i = 0; i < 20; i++) {
        yakl::timer_start("stencil-1d-3");
        yakl::c::parallel_for(
            yakl::c::Bounds<1>(1, N_1D + 1), YAKL_LAMBDA(int i) {
              yakl::SArray<double, 1, 3> a;
              for (int ii = -1; ii <= 1; ii++)
                a(ii + 1) = a_device1d(i + ii);
              b_device1d(i) = randq0 * a(1) + randq1 * a(0) + randq2 * a(2);
            });
        yakl::timer_stop("stencil-1d-3");
      }

      // stencil-1d-3-simd
      // real1d_device a_device1d("a_device1d", N + 2), b_device1d("b_device1d",
      // N + 2);
      yakl::c::parallel_for(
          yakl::c::Bounds<1>(1, N_1D + 1), YAKL_LAMBDA(int i) {
            a_device1d(i) = (double)rand();
            b_device1d(i) = 0.0;
          });
      for (int i = 0; i < 20; i++) {
        yakl::timer_start("stencil-1d-3-simd");
        yakl::c::parallel_for(
            yakl::c::Bounds<1>(1, N_1D / 8 + 1), YAKL_LAMBDA(int i) {
              yakl::SArray<yakl::simd::Pack<double, 8>, 1, 3> a;
              yakl::SArray<yakl::simd::Pack<double, 8>, 1, 3> b;
              for (int ii = -1; ii <= 1; ii++) {
                yakl::simd::iterate_over_pack(
                    [&](unsigned int ilane) {
                      a(ii + 1)(ilane) = a_device1d(min(N_1D, i * 8 + ilane) + ii);
                    },
                    yakl::simd::PackIterConfig<8, true>());
              }
              b(i) = randq0 * a(1) + randq1 * a(0) + randq2 * a(2);
            });
        yakl::timer_stop("stencil-1d-3-simd");
      }
    }
    // head-2d
    {
      printf("=========================================\n");
      printf("heat-2d\n");
      printf("=========================================\n");
      auto rnd = yakl::Random();
      real2d_host a_host("a_host", N + 2, N + 2),
          b_host("b_host", N + 2, N + 2);
      real2d_device a_device("a_device", N + 2, N + 2),
          b_device("b_device", N + 2, N + 2);

      parallel_for(
          Bounds<2>(1, N + 1, 1, N + 1),
          YAKL_LAMBDA(int i, int j) { a_device(i, j) = (double)rand(); });

      for (int i = 0; i < 20; i++) {
        yakl::timer_start("head-2d serial");
        for (int i = 1; i < N + 1; i++) {
          for (int j = 1; j < N + 1; j++) {
            b_host(i, j) =
                0.125 *
                    (a_host(i + 1, j) - 2.0 * a_host(i, j) + a_host(i - 1, j)) +
                0.125 *
                    (a_host(i, j + 1) - 2.0 * a_host(i, j) + a_host(i, j - 1)) +
                a_host(i, j);
          }
        }
        yakl::timer_stop("head-2d serial");

        yakl::timer_start("head-2d kernel");
        parallel_for(
            Bounds<2>({1, N + 1}, {1, N + 1}), YAKL_LAMBDA(int i, int j) {
              SArray<real, 2, 3, 3> a;
              for (int ii = -1; ii <= 1; ii++) {
                for (int jj = -1; jj <= 1; jj++) {
                  a(ii + 1, jj + 1) = a_device(i + ii, j + jj);
                }
              }
              b_device(i, j) = 0.125 * (a(2, 1) - 2.0 * a(1, 1) + a(0, 1)) +
                               0.125 * (a(1, 2) - 2.0 * a(1, 1) + a(1, 0)) +
                               a(1, 1);
            });
        yakl::timer_stop("head-2d kernel");
      }

      b_device.deep_copy_to(b_host);
    }

    // jacobi-2d
    {
      printf("=========================================\n");
      printf("jacobi-2d\n");
      printf("=========================================\n");
      real2d_host a_host("a_host", N + 2, N + 2),
          b_host("b_host", N + 2, N + 2);
      real2d_device a_device("a_device", N + 2, N + 2),
          b_device("b_device", N + 2, N + 2);

      parallel_for(
          Bounds<2>({0, N}, {0, N}), YAKL_LAMBDA(int i, int j) {
            a_device(i, j) = (static_cast<double>(j) / N);
          });

      a_device.deep_copy_to(a_host);

      for (int i = 0; i < 20; i++) {
        yakl::timer_start("jacobi-2d serial");
        for (int i = 2; i < N - 1; i++) {
          for (int j = 2; j < N - 1; j++) {
            b_host(i, j) =
                0.2 * (a_host(i, j) + a_host(i, j - 1) + a_host(i, j + 1) +
                       a_host(i + 1, j) + a_host(i - 1, j));
          }
        }
        yakl::timer_stop("jacobi-2d serial");

        yakl::timer_start("jacobi-2d kernel");
        parallel_for(
            Bounds<2>({2, N - 1}, {2, N - 1}), YAKL_LAMBDA(int i, int j) {
              SArray<real, 2, 3, 3> a;
              for (int ii = -1; ii <= 1; ii++) {
                for (int jj = -1; jj <= 1; jj++) {
                  a(ii + 1, jj + 1) = a_device(i + ii, j + jj);
                }
              }
              b_device(i, j) =
                  0.2 * (a(1, 1) + a(1, 0) + a(1, 2) + a(2, 1) + a(0, 1));
            });
        yakl::timer_stop("jacobi-2d kernel");
      }

      b_device.deep_copy_to(b_host);
    }

    // fdtd-2d
    {
      printf("=========================================\n");
      printf("fdtd-2d\n");
      printf("=========================================\n");

      constexpr int tmax = 128;
      real2d_host ex_host("ex_host", N, N + 1), ey_host("ey_host", N + 1, N),
          hz_host("hz_host", N, N);
      real2d_device ex_device("ex_device", N, N), ey_device("ey_device", N, N),
          hz_device("hz_device", N, N);

      parallel_for(
          Bounds<2>({0, N + 1}, {0, N + 1}), YAKL_LAMBDA(int i, int j) {
            if (i < N) {
              ey_device(i, j) = static_cast<double>(j) / N;
            }
            if (j < N) {
              ex_device(i, j) = 0.0;
            }
            if (i < N && j < N) {
              hz_device(i, j) = 0.0;
            }
          });

      ex_device.deep_copy_to(ex_host);
      ey_device.deep_copy_to(ey_host);
      hz_device.deep_copy_to(hz_host);

      for (int t = 0; t < tmax; t++) {
        yakl::timer_start("fdtd-2d serial");
        for (int j = 0; j < N; j++) {
          ey_host(0, j) = t;
        }
        for (int i = 1; i < N; i++) {
          for (int j = 0; j < N; j++) {
            ey_host(i, j) =
                ey_host(i, j) - 0.5 * (hz_host(i, j) - hz_host(i - 1, j));
          }
        }
        for (int i = 0; i < N; i++) {
          for (int j = 1; j < N; j++) {
            ex_host(i, j) =
                ex_host(i, j) - 0.5 * (hz_host(i, j) - hz_host(i, j - 1));
          }
        }
        for (int i = 0; i < N; i++) {
          for (int j = 0; j < N; j++) {
            hz_host(i, j) =
                hz_host(i, j) - 0.7 * (ex_host(i, j + 1) - ex_host(i, j) +
                                       ey_host(i + 1, j) - ey_host(i, j));
          }
        }
        yakl::timer_stop("fdtd-2d serial");
      }
      printf("-----------------------------------------\n");
      printf("YAKL parallel_for\n");
      printf("-----------------------------------------\n");
      for (int t = 0; t < tmax; t++) {
        yakl::timer_start("fdtd-2d kernel");
        parallel_for(Bounds<1>(N), YAKL_LAMBDA(int i) { ey_device(0, i) = t; });
        parallel_for(
            Bounds<2>({0, N}, {0, N}),
            YAKL_LAMBDA(int i, int j) {
              if (i > 1) {
                ey_device(i, j) = ey_device(i, j) -
                                  0.5 * (hz_device(i, j) - hz_device(i - 1, j));
              }
              if (j > 1) {
                ex_device(i, j) = ex_device(i, j) -
                                  0.5 * (hz_device(i, j) - hz_device(i, j - 1));
              }
            },
            yakl::LaunchConfig<1024, false>());
        parallel_for(
            Bounds<2>({0, N}, {0, N}),
            YAKL_LAMBDA(int i, int j) {
              if (i > 0 && j > 0) {
                hz_device(i, j) = hz_device(i, j) -
                                  0.7 * (ex_device(i, j + 1) - ex_device(i, j) +
                                         ey_device(i + 1, j) - ey_device(i, j));
              }
            },
            yakl::LaunchConfig<1024, false>());
        yakl::timer_stop("fdtd-2d kernel");
      }
    }
  }
  yakl::finalize();
}