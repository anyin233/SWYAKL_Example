

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

void test_call_depth(int depth, int &res, const int N) {
  if (depth == N)
    return;
  else {
    res += depth * res;
    test_call_depth(depth + 1, res, N);
  }
}

int main(int argc, char **argv) {
  // if (argc != 2) {
  //   printf("Usage: %s <N>\n", argv[0]);
  //   exit(1);
  // }

  const int N = 1000;
  // const int N_3D = atoi(argv[2]);
  std::cout << "N = " << N << std::endl;
  yakl::init();
  {
    real1d_device res_arr("res", 1024);
    parallel_for(
        "Test Depth", Bounds<1>(1024), YAKL_LAMBDA(int i) {
          int res = 1;
          test_call_depth(1, res, N);
          res_arr(i) = res;
        });
    for (int i = 0; i < 128; i++) {
      std::cout << res_arr(i) << std::endl;
    }
  }
  yakl::finalize();
}