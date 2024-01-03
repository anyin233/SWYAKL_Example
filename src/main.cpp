#include <iostream>
#include "athread.h"
#include "YAKL.h"
#include "sw_helper.h"
#include <chrono>

const size_t N = 1 << 30;

// #pragma swuc push host
int main() {
  athread_init();
  yakl::init();
{
  // printf("Initialized\n");
  yakl::Array<int, 1, yakl::memHost, yakl::styleC> a("a", N), b("b", N), c("c", N);
  // printf("Array Crated\n");

  yakl::c::parallel_for("Initialize", yakl::c::Bounds<1>(N), YAKL_LAMBDA(int i) {
    a(i) = i;
    b(i) = i + 1;
    c(i) = 0;
  });

  auto start = std::chrono::system_clock::now();
  for (int i = 0; i < N; i ++) {
    c(i) = a(i) * b(i);
  }
  auto end = std::chrono::system_clock::now();
  auto elapesd_CPE = std::chrono::duration<double>(end - start);

  for (int i = 0; i < N; i ++) {
    if (c(i) != a(i) * b(i)) {
      printf("Index %d is error, expect %d, got %d\n", i, a(i) * b(i), c(i));
      break;
    }    
  }
  // printf("Kernel Finished\n");
  std::cout << "Kernel Finished\n";

  start = std::chrono::system_clock::now();
  
  yakl::c::parallel_for("Compute", yakl::c::Bounds<1>(N), YAKL_LAMBDA(int i) {
    c(i) = a(i) * b(i);
  });
  end = std::chrono::system_clock::now();
  auto elapesd_MPE = std::chrono::duration<double>(end - start);

  printf("CPE version costs %lf\nMPE version costs %lf\n", elapesd_CPE.count(), elapesd_MPE.count());
}

  yakl::finalize();
  // std::cout << "Program Exiting" << std::endl;
  // SW_BKPT(finalize);
}