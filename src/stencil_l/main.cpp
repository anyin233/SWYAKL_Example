

#include "YAKL.h"
#include "YAKL_LaunchConfig.h"
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
        yakl::timer_start("stencil-2d-5 serial");
        for (int i = 1; i < N + 1; i ++) {
          for (int j = 1; j < N + 1; j ++) {
            b_device2d(i, j) = randq0 * a_device2d(i, j) + randq1 * a_device2d(i - 1, j) +
                               randq2 * a_device2d(i + 1, j) + randq3 * a_device2d(i, j - 1) +
                               randq4 * a_device2d(i, j + 1);
          }
        }
        yakl::timer_stop("stencil-2d-5 serial");
        
        yakl::timer_start("stencil-2d-5 kernel");
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
        yakl::timer_stop("stencil-2d-5 kernel");

        yakl::timer_start("stencil-2d-5 simd");
        yakl::c::parallel_for(
            yakl::c::Bounds<2>({1, N + 1}, {1, (N - 1) / 8 + 1 + 1}),
            YAKL_LAMBDA(int i, int j) {
              auto j_start = j * 8 - 8;
              auto j_end = min(j_start + 8, N + 1);
              yakl::SArray<yakl::simd::Pack<double, 8>, 2, 3, 3> a_packs;
              yakl::SArray<yakl::simd::Pack<double, 8>, 1, 5> rand_packs;
              for (int ii = -1; ii < 1; ii ++) {
                for (int jj = -1; jj <= 1; jj ++) {
                  for (int vi = 0; vi < 8 && vi + j_start < N + 1; vi ++) {
                    a_packs(ii + 1, jj + 1)(vi) = a_device2d(i + ii, vi + j_start + jj);
                  }
                }
              }
              rand_packs(0) = randq0;
              rand_packs(1) = randq1;
              rand_packs(2) = randq2;
              rand_packs(3) = randq3;
              rand_packs(4) = randq4;
              a_packs(1, 1) *= rand_packs(0);
              a_packs(0, 1) *= rand_packs(1);
              a_packs(2, 1) *= rand_packs(2);
              a_packs(1, 0) *= rand_packs(3);
              a_packs(1, 2) *= rand_packs(4);
              a_packs(1, 2) += a_packs(1, 1);
              a_packs(1, 2) += a_packs(0, 1);
              a_packs(1, 2) += a_packs(2, 1);
              a_packs(1, 2) += a_packs(1, 0);
              for (int vi = 0; vi < 8 && vi + j_start < N + 1; vi ++) {
                b_device2d(i, vi + j_start) = a_packs(1, 2)(vi);
              }
            });
        yakl::timer_stop("stencil-2d-5 simd");
        
      }

      yakl::c::parallel_for(
          yakl::c::Bounds<2>({1, N + 1}, {1, N + 1}),
          YAKL_LAMBDA(int i, int j) {
            a_device2d(i, j) = (double)rand();
            b_device2d(i, j) = 0.0;
          });

      for (int i = 0; i < 20; i++) {
        yakl::timer_start("stencil-2d-9 serial");
        for (int i = 1; i < N + 1; i ++) {
          for (int j = 1; j < N + 1; j ++) {
            b_device2d(i, j) = randq0 * a_device2d(i, j) + randq1 * a_device2d(i - 1, j) +
                               randq2 * a_device2d(i + 1, j) + randq3 * a_device2d(i, j - 1) +
                               randq4 * a_device2d(i, j + 1) + randq5 * a_device2d(i - 1, j - 1) +
                               randq6 * a_device2d(i + 1, j + 1) + randq7 * a_device2d(i - 1, j + 1) +
                               randq8 * a_device2d(i + 1, j - 1);
          }
        }
        yakl::timer_stop("stencil-2d-9 serial");

        yakl::timer_start("stencil-2d-9 kernel");
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
            }, yakl::LaunchConfig<512 * 8, false>());
        yakl::timer_stop("stencil-2d-9 kernel");

        yakl::timer_start("stencil-2d-9 simd");
        yakl::c::parallel_for(
            yakl::c::Bounds<2>({1, N + 1}, {1, (N - 1)/ 8 + 1 + 1}),
            YAKL_LAMBDA(int i, int j) {
              SArray<yakl::simd::Pack<double, 8>, 2, 3, 3> a_packs;
              SArray<yakl::simd::Pack<double, 8>, 1, 9> rand_packs;

              auto j_start = 1 + j * 8;
              auto j_end = min(N + 1, j_start + 8);
              for (int ii = -1; ii <= 1; ii ++) {
                for (int jj = -1; jj <= 1; jj ++) {
                  for (int vi = 0; vi < 8 && vi + j_start < N + 1; vi ++) {
                    a_packs(ii + 1, jj + 1)(vi) = a_device2d(i + ii, vi + j * 8 - 8 + jj);
                  }
                }
              }
              rand_packs(0) = randq0;
              rand_packs(1) = randq1;
              rand_packs(2) = randq2;
              rand_packs(3) = randq3;
              rand_packs(4) = randq4;
              rand_packs(5) = randq5;
              rand_packs(6) = randq6;
              rand_packs(7) = randq7;
              rand_packs(8) = randq8;

              rand_packs(0) *= a_packs(1, 1);
              rand_packs(1) *= a_packs(0, 1);
              rand_packs(2) *= a_packs(2, 1);
              rand_packs(3) *= a_packs(1, 0);
              rand_packs(4) *= a_packs(1, 2);
              rand_packs(5) *= a_packs(0, 0);
              rand_packs(6) *= a_packs(0, 2);
              rand_packs(7) *= a_packs(2, 0);
              rand_packs(8) *= a_packs(2, 2);
              rand_packs(0) += rand_packs(1);
              rand_packs(0) += rand_packs(2);
              rand_packs(0) += rand_packs(3);
              rand_packs(0) += rand_packs(4);
              rand_packs(0) += rand_packs(5);
              rand_packs(0) += rand_packs(6);
              rand_packs(0) += rand_packs(7);
              rand_packs(0) += rand_packs(8);

              for (int vi = 0; vi < 8 && vi + j_start < N + 1; vi ++) {
                b_device2d(i, vi + j * 8 - 8) = rand_packs(0)(vi);
              }
            });
        yakl::timer_stop("stencil-2d-9 simd");
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
        yakl::timer_start("stencil-3d-7 serial");
        
        for (int i = 1; i < N + 1; i ++) {
          for (int j = 1; j < N + 1; j ++) {
            for (int k = 1; k < N + 1; k ++) {
              b_device3d(i, j, k) = randq0 * a_device3d(i, j, k) + randq1 * a_device3d(i - 1, j, k) +
                                    randq2 * a_device3d(i + 1, j, k) + randq3 * a_device3d(i, j - 1, k) +
                                    randq4 * a_device3d(i, j + 1, k) + randq5 * a_device3d(i, j, k - 1) +
                                    randq6 * a_device3d(i, j, k + 1);
            }
          }
        }
        yakl::timer_stop("stencil-3d-7 serial");

        yakl::timer_start("stencil-3d-7 kernel");
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
        yakl::timer_stop("stencil-3d-7 kernel");

        yakl::timer_start("stencil-3d-7 simd");
        yakl::c::parallel_for(
            yakl::c::Bounds<3>({1, N + 1}, {1, N + 1}, {1, N + 1}),
            YAKL_LAMBDA(int i, int j, int k) {
              yakl::simd::Pack<double, 8> a_pack;
              yakl::simd::Pack<double, 8> rand_pack;

              a_pack(0) = a_device3d(i + 1, j + 1, k + 1);
              a_pack(1) = a_device3d(i + 0, j + 1, k + 1);
              a_pack(2) = a_device3d(i + 2, j + 1, k + 1);
              a_pack(3) = a_device3d(i + 1, j + 0, k + 1);
              a_pack(4) = a_device3d(i + 1, j + 2, k + 1);
              a_pack(5) = a_device3d(i + 1, j + 1, k + 0);
              a_pack(6) = a_device3d(i + 1, j + 1, k + 2);
              a_pack(7) = 0;
              rand_pack(0) = randq0;
              rand_pack(1) = randq1;
              rand_pack(2) = randq2;
              rand_pack(3) = randq3;
              rand_pack(4) = randq4;
              rand_pack(5) = randq5;
              rand_pack(6) = randq6;
              rand_pack(7) = 0;
              rand_pack *= a_pack;
              b_device3d(i, j, k) = rand_pack.sum();
            });
        yakl::timer_stop("stencil-3d-7 simd");
      }

      yakl::c::parallel_for(
          yakl::c::Bounds<3>({1, N + 1}, {1, N + 1}, {1, N + 1}),
          YAKL_LAMBDA(int i, int j, int k) {
            a_device3d(i, j, k) = (double)rand();
            b_device3d(i, j, k) = 0.0;
          });
      for (int i = 0; i < 20; i++) {
        yakl::timer_start("stencil-3d-27 serial");
        for (int i = 1; i < N + 1; i ++) {
          for (int j = 1; j < N + 1; j ++) {
            for (int k = 1; k < N + 1; k ++) {
              b_device3d(i, j, k) = randq0 * a_device3d(i, j, k) + randq1 * a_device3d(i - 1, j, k) +
                                    randq2 * a_device3d(i + 1, j, k) + randq3 * a_device3d(i, j - 1, k) +
                                    randq4 * a_device3d(i, j + 1, k) + randq5 * a_device3d(i, j, k - 1) +
                                    randq6 * a_device3d(i, j, k + 1) + randq7 * a_device3d(i - 1, j - 1, k) +
                                    randq8 * a_device3d(i + 1, j - 1, k) + randq9 * a_device3d(i - 1, j + 1, k) +
                                    randq10 * a_device3d(i + 1, j + 1, k) + randq11 * a_device3d(i - 1, j, k - 1) +
                                    randq12 * a_device3d(i + 1, j, k - 1) + randq13 * a_device3d(i - 1, j, k + 1) +
                                    randq14 * a_device3d(i + 1, j, k + 1) + randq15 * a_device3d(i, j - 1, k - 1) +
                                    randq16 * a_device3d(i, j + 1, k - 1) + randq17 * a_device3d(i, j - 1, k + 1) +
                                    randq18 * a_device3d(i, j + 1, k + 1) + randq19 * a_device3d(i - 1, j - 1, k - 1) +
                                    randq20 * a_device3d(i - 1, j + 1, k - 1) + randq21 * a_device3d(i + 1, j - 1, k - 1) +
                                    randq22 * a_device3d(i + 1, j + 1, k - 1) + randq23 * a_device3d(i - 1, j - 1, k + 1) + 
                                    randq24 * a_device3d(i - 1, j + 1, k + 1) + randq25 * a_device3d(i + 1, j - 1, k + 1) +
                                    randq26 * a_device3d(i + 1, j + 1, k + 1); 
            }
          }
        }
        yakl::timer_stop("stencil-3d-27 serial");

        yakl::timer_start("stencil-3d-27 kernel");
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
        yakl::timer_stop("stencil-3d-27 kernel");

        yakl::timer_start("stencil-3d-27 simd");
        yakl::c::parallel_for(
            yakl::c::Bounds<3>({1, N + 1}, {1, N + 1}, {0, (N - 1) / 8 + 1}),
            YAKL_LAMBDA(int i, int j, int k) {
              yakl::SArray<yakl::simd::Pack<double, 8>, 3, 3, 3, 3> a_packs;
              yakl::SArray<yakl::simd::Pack<double, 8>, 1, 27> rand_packs;

              auto k_start = k * 8 + 1;
              auto k_end = min(k_start + 8, N + 1);
              for (int ii = -1; ii <= 1; ii ++) {
                for (int jj = -1; jj <= 1; jj ++) {
                  for (int kk = -1; kk <= 1; kk ++) {
                    for (int vi = 0; vi < 8 && vi + k_start < N + 1; vi ++) {
                      a_packs(ii + 1, jj + 1, kk + 1)(vi) = a_device3d(i + ii, j + jj, vi + k_start + kk);
                    }
                    // a_packs(ii + 1, jj + 1, kk + 1).load(&a_device3d(i + ii, j + jj, k_start + kk));
                  }
                }
              }
              rand_packs(0) = randq0;
              rand_packs(1) = randq1;
              rand_packs(2) = randq2;
              rand_packs(3) = randq3;
              rand_packs(4) = randq4;
              rand_packs(5) = randq5;
              rand_packs(6) = randq6;
              rand_packs(7) = randq7;
              rand_packs(8) = randq8;
              rand_packs(9) = randq9;
              rand_packs(10) = randq10;
              rand_packs(11) = randq11;
              rand_packs(12) = randq12;
              rand_packs(13) = randq13;
              rand_packs(14) = randq14;
              rand_packs(15) = randq15;
              rand_packs(16) = randq16;
              rand_packs(17) = randq17;
              rand_packs(18) = randq18;
              rand_packs(19) = randq19;
              rand_packs(20) = randq20;
              rand_packs(21) = randq21;
              rand_packs(22) = randq22;
              rand_packs(23) = randq23;
              rand_packs(24) = randq24;
              rand_packs(25) = randq25;
              rand_packs(26) = randq26;

              rand_packs(0) *= a_packs(1, 1, 1);
              auto x = rand_packs(0) * a_packs(1, 1, 1);
              rand_packs(0).ma(rand_packs(1), a_packs(0, 1, 1));
              rand_packs(0).ma(rand_packs(2), a_packs(2, 1, 1));
              rand_packs(0).ma(rand_packs(3), a_packs(1, 0, 1));
              rand_packs(0).ma(rand_packs(4), a_packs(1, 2, 1));
              rand_packs(0).ma(rand_packs(5), a_packs(1, 1, 0));
              rand_packs(0).ma(rand_packs(6), a_packs(1, 1, 2));
              rand_packs(0).ma(rand_packs(7), a_packs(0, 0, 1));
              rand_packs(0).ma(rand_packs(8), a_packs(0, 2, 1));
              rand_packs(0).ma(rand_packs(9), a_packs(2, 0, 1));
              rand_packs(0).ma(rand_packs(10), a_packs(2, 2, 1));
              rand_packs(0).ma(rand_packs(11), a_packs(0, 1, 0));
              rand_packs(0).ma(rand_packs(12), a_packs(0, 1, 2));
              rand_packs(0).ma(rand_packs(13), a_packs(2, 1, 0));
              rand_packs(0).ma(rand_packs(14), a_packs(2, 1, 2));
              rand_packs(0).ma(rand_packs(15), a_packs(1, 0, 0));
              rand_packs(0).ma(rand_packs(16), a_packs(1, 0, 2));
              rand_packs(0).ma(rand_packs(17), a_packs(1, 2, 0));
              rand_packs(0).ma(rand_packs(18), a_packs(1, 2, 2));
              rand_packs(0).ma(rand_packs(19), a_packs(0, 0, 0));
              rand_packs(0).ma(rand_packs(20), a_packs(0, 0, 2));
              rand_packs(0).ma(rand_packs(21), a_packs(0, 2, 0));
              rand_packs(0).ma(rand_packs(22), a_packs(0, 2, 2));
              rand_packs(0).ma(rand_packs(23), a_packs(2, 0, 0));
              rand_packs(0).ma(rand_packs(24), a_packs(2, 0, 2));
              rand_packs(0).ma(rand_packs(25), a_packs(2, 2, 0));
              rand_packs(0).ma(rand_packs(26), a_packs(2, 2, 2));

              for (int vi = 0; vi < 8 && vi + k_start < N + 1; vi ++) {
                b_device3d(i, j, vi + k_start) = rand_packs(0)(vi);
              }
            }, yakl::LaunchConfig<128, false>());
        yakl::timer_stop("stencil-3d-27 simd");
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
        yakl::timer_start("stencil-1d-3 serial");
        for (int i = 1; i < N_1D + 1; i ++) {
          b_device1d(i) = randq0 * a_device1d(i) + randq1 * a_device1d(i - 1) + randq2 * a_device1d(i + 1);
        }
        yakl::timer_stop("stencil-1d-3 serial");
      }

      for (int i = 0; i < 20; i++) {
        yakl::timer_start("stencil-1d-3 kernel");
        yakl::c::parallel_for(
            yakl::c::Bounds<1>(1, N_1D + 1), YAKL_LAMBDA(int i) {
              yakl::SArray<double, 1, 3> a;
              for (int ii = -1; ii <= 1; ii++)
                a(ii + 1) = a_device1d(i + ii);
              b_device1d(i) = randq0 * a(1) + randq1 * a(0) + randq2 * a(2);
            });
        yakl::timer_stop("stencil-1d-3 kernel");
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
        yakl::timer_start("stencil-1d-3 simd");
        yakl::c::parallel_for(
            yakl::c::Bounds<1>(1, N_1D / 8 + 1), YAKL_LAMBDA(int i) {
              yakl::SArray<yakl::simd::Pack<double, 8>, 1, 3> a;
              // yakl::SArray<yakl::simd::Pack<double, 8>, 1, 3> b;
              yakl::SArray<yakl::simd::Pack<double, 8>, 1, 3> s_vals;

              auto i_start = i * 8;
              auto i_end = min(N_1D, i_start + 8);
              for (auto vi = 0; vi < 8 && vi + i_start < i_end; vi++) {
                auto real_i = vi + i_start;
                for (int ii = -1; ii <= 1; ii++) {
                  a(ii + 1)(vi) = a_device1d(real_i + ii);
                }
                s_vals(0)(vi) = randq0;
                s_vals(1)(vi) = randq1;
                s_vals(2)(vi) = randq2;
              }

              s_vals(0) *= a(1);
              s_vals(1) *= a(0);
              s_vals(2) *= a(2);
              s_vals(2) += s_vals(1);
              s_vals(2) += s_vals(0);

              yakl::simd::iterate_over_pack(
                  [&](unsigned int ilane) {
                    b_device1d(i * 8 + ilane) = s_vals(i)(ilane);
                  },
                  yakl::simd::PackIterConfig<8, false>());
            });
        yakl::timer_stop("stencil-1d-3 simd");
      }
    }
    // heat-2d
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
        yakl::timer_start("heat-2d serial");
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
        yakl::timer_stop("heat-2d serial");

        yakl::timer_start("heat-2d kernel");
        parallel_for(
            Bounds<2>({1, N + 1}, {1, N + 1}),
            YAKL_LAMBDA(int i, int j) {
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
        yakl::timer_stop("heat-2d kernel");

        yakl::timer_start("heat-2d simd");
        parallel_for(
            Bounds<2>({1, N + 1}, {1, 1 + ((N - 1) / 8 + 1)}),
            YAKL_LAMBDA(int i, int j) {
              SArray<yakl::simd::Pack<double, 8>, 2, 3, 3> a_packs;
              yakl::simd::Pack<double, 8> s_val0;
              yakl::simd::Pack<double, 8> s_val1;
              yakl::simd::Pack<double, 8> s_val2;
              auto j_start = 1 + j * 8;
              auto j_end = min(N + 1, j_start + 8);

              for (int ii = -1; ii <= 1; ii++) {
                for (int jj = -1; jj <= 1; jj++) {
                  for (int vi = 0; vi < 8 && vi + j_start < j_end; vi++) {
                    auto real_j = vi + j_start;
                    a_packs(ii + 1, jj + 1)(vi) = a_device(i + ii, real_j + jj);
                  }
                }
              }

              s_val0 = 0.125;
              s_val1 = 2;
              s_val2 = 0.125;

              s_val1 *= a_packs(1, 1);
              a_packs(2, 1) -= s_val1;
              a_packs(1, 2) -= s_val1;

              a_packs(2, 1) += a_packs(0, 1);
              s_val0 *= a_packs(2, 1);

              a_packs(1, 2) += a_packs(1, 0);
              s_val2 *= a_packs(1, 2);

              s_val2 += s_val0;
              s_val2 += a_packs(1, 1);

              for (int vi = 0; vi < 8 && vi + j_start < j_end; vi++) {
                b_device(i, vi + j_start) = s_val2(vi);
              }
            });
        yakl::timer_stop("heat-2d simd");
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
            Bounds<2>({2, N - 1}, {2, N - 2}),
            YAKL_LAMBDA(int i, int j) {
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

        yakl::timer_start("jacobi-2d simd");
        parallel_for(
            Bounds<2>({2, N - 1}, {2, ((N - 4) / 8 + 3)}),
            YAKL_LAMBDA(int i, int j) {
              auto j_start = 2 + j * 8;
              auto j_end = min(N - 1, j_start + 8);

              SArray<yakl::simd::Pack<double, 8>, 2, 3, 3> a_packs;
              yakl::simd::Pack<double, 8> s_val0;
              for (auto vi = 0; vi < 8 && vi + j_start < j_end; vi++) {
                for (int ii = -1; ii <= 1; ii++) {
                  for (int jj = -1; jj <= 1; jj++) {
                    a_packs(ii + 1, jj + 1)(vi) =
                        a_device(i + ii, vi + j_start + jj);
                  }
                }
                s_val0 = 0.2;
              }

              a_packs(1, 1) += a_packs(1, 0);
              a_packs(1, 1) += a_packs(1, 2);
              a_packs(1, 1) += a_packs(2, 1);
              a_packs(1, 1) += a_packs(0, 1);
              a_packs(1, 1) *= s_val0;

              for (auto vi = 0; vi < 8 && vi + j_start < j_end; vi++) {
                b_device(i, vi + j_start) = a_packs(1, 1)(vi);
              }
            });
        yakl::timer_stop("jacobi-2d simd");
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
      printf("-----------------------------------------\n");
      printf("YAKL parallel_for simd\n");
      printf("-----------------------------------------\n");
      for (int t = 0; t < tmax; t++) {
        yakl::timer_start("fdtd-2d simd");
        parallel_for(Bounds<1>(N), YAKL_LAMBDA(int i) { ey_device(0, i) = t; });
        parallel_for(
            Bounds<2>({0, N}, {0, (N - 1) / 8 + 1}),
            YAKL_LAMBDA(int i, int j) {
              auto j_start = j * 8;
              auto j_end = min(N, j_start + 8);
              SArray<yakl::simd::Pack<double, 8>, 1, 3> hz_packs;
              yakl::simd::Pack<double, 8> ey_pack;
              yakl::simd::Pack<double, 8> ex_pack;
              yakl::simd::Pack<double, 8> s_val0;

              for (auto vi = 0; vi < N && vi + j_start < j_end; vi++) {
                hz_packs(0)(vi) = hz_device(i, vi + j_start);
                if (i > 1) {
                  hz_packs(1)(vi) = hz_device(i - 1, vi + j_start);
                  ey_pack(vi) = ey_device(i, vi + j_start);
                }
                if (j > 1) {
                  hz_packs(2)(vi) = hz_device(i, vi + j_start - 1);
                  ex_pack(vi) = ex_device(i, vi + j_start);
                }
                s_val0(vi) = 0.5;
              }

              if (i > 1) {
                hz_packs(1) -= hz_packs(0);
                hz_packs(1) *= s_val0;
                ey_pack += hz_packs(1);
                // ey_device(i, j) = ey_device(i, j) -
                //                   0.5 * (hz_device(i, j) - hz_device(i - 1,
                //                   j));
                for (int vi = 0; vi < N && vi + j_start < j_end; vi++) {
                  ey_device(i, vi + j_start) = ey_pack(vi);
                }
              }
              if (j > 1) {
                hz_packs(2) -= hz_packs(0);
                hz_packs(2) *= s_val0;
                ex_pack += hz_packs(2);
                // ex_device(i, j) = ex_device(i, j) -
                //                   0.5 * (hz_device(i, j) - hz_device(i, j -
                //                   1));
                for (int vi = 0; vi < N && vi + j_start < j_end; vi++) {
                  ex_device(i, vi + j_start) = ex_pack(vi);
                }
              }
            },
            yakl::LaunchConfig<1024, false>());
        parallel_for(
            Bounds<2>({0, N}, {0, (N - 1) / 8 + 1}),
            YAKL_LAMBDA(int i, int j) {
              if (i > 0 && j > 0) {
                SArray<yakl::simd::Pack<double, 8>, 1, 2> ex_packs;
                SArray<yakl::simd::Pack<double, 8>, 1, 2> ey_packs;
                yakl::simd::Pack<double, 8> hz_packs;
                yakl::simd::Pack<double, 8> s_val0;
                auto j_start = j * 8;
                auto j_end = min(N, j_start + 8);
                for (auto vi = 0; vi < N && vi + j_start < j_end; vi++) {
                  hz_packs(vi) = hz_device(i, vi + j_start);

                  ex_packs(0)(vi) = ex_device(i, vi + j_start);
                  ex_packs(1)(vi) = ex_device(i, vi + j_start + 1);

                  ey_packs(0)(vi) = ey_device(i, vi + j_start);
                  ey_packs(1)(vi) = ey_device(i, vi + j_start + 1);
                  s_val0(vi) = 0.7;
                }

                ex_packs(1) -= ex_packs(0);
                ex_packs(1) += ey_packs(1);
                ex_packs(1) -= ey_packs(0);
                ex_packs(1) *= s_val0;
                hz_packs -= ex_packs(1);

                for (auto vi = 0; vi < N && vi + j_start < j_end; vi++) {
                  hz_device(i, vi + j_start) = hz_packs(vi);
                }
                // hz_device(i, j) = hz_device(i, j) -
                //                   0.7 * (ex_device(i, j + 1) - ex_device(i,
                //                   j) +
                //                          ey_device(i + 1, j) - ey_device(i,
                //                          j));
              }
            },
            yakl::LaunchConfig<1024, false>());
        yakl::timer_stop("fdtd-2d simd");
      }
    }
  }
  yakl::finalize();
}