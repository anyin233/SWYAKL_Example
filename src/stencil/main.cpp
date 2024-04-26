#include "YAKL.h"
#include "YAKL_timers.h"
#include "intrinsics/YAKL_intrinsics_sum.h"

using yakl::Array;
using yakl::memDevice;
using yakl::memHost;
using yakl::styleC;
using yakl::c::Bounds;
using yakl::c::parallel_for;

using yakl::SArray;
using real = double;

typedef Array<real, 1, memDevice, styleC> real1d_device;
typedef Array<real, 1, memHost, styleC> real1d_host;
typedef Array<real, 2, memDevice, styleC> real2d_device;
typedef Array<real, 2, memHost, styleC> real2d_host;

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s <N> <NY>\n", argv[0]);
    exit(1);
  }
  const int N = atoi(argv[1]);
  const int NY = atoi(argv[2]);

  printf("N = %d\n", N);
  yakl::init();
  {
    // head-2d
    {
      printf("=========================================\n");
      printf("head-2d\n");
      printf("=========================================\n");
      auto rnd = yakl::Random();
      real2d_host a_host("a_host", N + 2, N + 2),
          b_host("b_host", N + 2, N + 2);
      real2d_device a_device("a_device", N + 2, N + 2),
          b_device("b_device", N + 2, N + 2);

      parallel_for(
          Bounds<2>(1, N + 1, 1, N + 1),
          YAKL_LAMBDA(int i, int j) { a_device(i, j) = rnd.genFP<double>(); });

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
      real2d_host ex_host("ex_host", N, NY + 1), ey_host("ey_host", N + 1, NY),
          hz_host("hz_host", N, NY);
      real2d_device ex_device("ex_device", N, NY),
          ey_device("ey_device", N, NY), hz_device("hz_device", N, NY);

      parallel_for(
          Bounds<2>({0, N + 1}, {0, NY + 1}), YAKL_LAMBDA(int i, int j) {
            if (i < N) {
              ey_device(i, j) = static_cast<double>(j) / N;
            }
            if (j < NY) {
              ex_device(i, j) = 0.0;
            }
            if (i < N && j < NY) {
              hz_device(i, j) = 0.0;
            }
          });

      ex_device.deep_copy_to(ex_host);
      ey_device.deep_copy_to(ey_host);
      hz_device.deep_copy_to(hz_host);

      for (int t = 0; t < tmax; t++) {
        yakl::timer_start("fdtd-2d serial");
        for (int j = 0; j < NY; j++) {
          ey_host(0, j) = t;
        }
        for (int i = 1; i < N; i++) {
          for (int j = 0; j < NY; j++) {
            ey_host(i, j) =
                ey_host(i, j) - 0.5 * (hz_host(i, j) - hz_host(i - 1, j));
          }
        }
        for (int i = 0; i < N; i++) {
          for (int j = 1; j < NY; j++) {
            ex_host(i, j) =
                ex_host(i, j) - 0.5 * (hz_host(i, j) - hz_host(i, j - 1));
          }
        }
        for (int i = 0; i < N; i++) {
          for (int j = 0; j < NY; j++) {
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
        parallel_for(
            Bounds<1>(NY), YAKL_LAMBDA(int i) { ey_device(0, i) = t; });
        parallel_for(
            Bounds<2>({0, N}, {0, NY}), YAKL_LAMBDA(int i, int j) {
              if (i > 1) {
                ey_device(i, j) = ey_device(i, j) - 0.5 * (hz_device(i, j) - hz_device(i - 1, j));
              }
              if (j > 1) {
                ex_device(i, j) = ex_device(i, j) - 0.5 * (hz_device(i, j) - hz_device(i, j - 1));
              }
            }, yakl::LaunchConfig<1024, false>());
        parallel_for(
            Bounds<2>({0, N}, {0, NY}), YAKL_LAMBDA(int i, int j) {
              if (i > 0 && j > 0) {
                hz_device(i, j) = hz_device(i, j) -
                                  0.7 * (ex_device(i, j + 1) - ex_device(i, j) +
                                         ey_device(i + 1, j) - ey_device(i, j));
              }
            }, yakl::LaunchConfig<1024, false>());
        yakl::timer_stop("fdtd-2d kernel");
      }
    }
  }
  yakl::finalize();
}