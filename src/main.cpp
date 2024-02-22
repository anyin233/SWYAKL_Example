#include "sw_helper.h"
#include <ios>
#include <iostream>
#include "athread.h"
#include "YAKL.h"
#include <chrono>
#include <iomanip>
#include <istream>

const size_t N = 1 << 30;
using C1Array = yakl::Array<int, 1, yakl::memHost, yakl::styleC>;

class MulFunctor {
  C1Array &a, &b, &c;
#ifdef __sw_slave__
  yakl::sw::LDMManager<int, 1> *la, *lb, *lc;
#endif // __sw_slave__
public:
  MulFunctor(C1Array &a, C1Array &b, C1Array &c): a(a), b(b), c(c) {}

#ifdef __sw_slave__
  // void enable_ldm() const {
  //   la = new yakl::sw::LDMManager<int, 1>(64);
  //   lb = new yakl::sw::LDMManager<int, 1>(64);
  //   lc = new yakl::sw::LDMManager<int, 1>(64);
  // }

  // void disable_ldm() const {
  //   delete la;
  //   delete lb;
  //   delete lc;
  // }  
#endif
  void operator()(int i) const {
    c(i) = a(i) * b(i);
  }

};



void print_size_of_Internal() {
  std::cout << "\n\nIn current environment\n"
            << "size_t = " << sizeof(size_t) << " int = " << sizeof(int) << " long = " << sizeof(long) << std::endl
            << "fake_std::mutex = " << sizeof(yakl::fake_std::mutex) << " fake_std::lock_guard = " << sizeof(yakl::fake_std::lock_guard<yakl::fake_std::mutex>) << std::endl;
  std::cout << "Sizeof YAKL_Instance\t" << sizeof(yakl::get_yakl_instance()) << std::endl;
  std::cout 
       << "Sizeof YAKL_Instance::pool           " << sizeof(yakl::get_yakl_instance().pool) << std::endl
       << "Sizeof YAKL_Instance::mutex          " << sizeof(yakl::get_yakl_instance().yakl_mtx) << std::endl
       << "Sizeof YAKL_Instance::mutex          " << sizeof(yakl::get_yakl_instance().yakl_final_mtx) << std::endl
       << "Sizeof YAKL_Instance::is_initialized " << sizeof(yakl::get_yakl_instance().yakl_is_initialized) << std::endl
       << "Sizeof YAKL_Instance functions\n"
       << "timer_init_func      " << sizeof(yakl::get_yakl_instance().timer_init_func) << std::endl
       << "timer_finalize_func  " << sizeof(yakl::get_yakl_instance().timer_finalize_func) << std::endl
       << "timer_start_func     " << sizeof(yakl::get_yakl_instance().timer_start_func) << std::endl
       << "timer_stop_func      " << sizeof(yakl::get_yakl_instance().timer_stop_func) << std::endl
       << "alloc_device_func    " << sizeof(yakl::get_yakl_instance().alloc_device_func) << std::endl
       << "free_device_func     " << sizeof(yakl::get_yakl_instance().free_device_func) << std::endl
       << "Sizeof YAKL_Instance::device_allocators_are_default  " << sizeof(yakl::get_yakl_instance().device_allocators_are_default) << std::endl
       << "Sizeof YAKL_Instance::pool_enabled                   " << sizeof(yakl::get_yakl_instance().pool_enabled) << std::endl
       << "Sizeof YAKL_Instance::finalize_callbacks             " << sizeof(yakl::get_yakl_instance().finalize_callbacks) << std::endl;
}

[[gnu::kernel]] void print_size() {
  if (_PEN == 0) {
    print_size_of_Internal();
  }
}

// #pragma swuc push host
int main() {
  // std::ios_base::sync_with_stdio(false);
  athread_init();
  // SW_BKPT(init);
  yakl::init();
{
  // printf("Initialized\n");
  print_size_of_Internal();
  yakl::Array<int, 1, yakl::memHost, yakl::styleC> a("a", N), b("b", N), c("c", N);
  // printf("Array Crated\n");
  print_size();
  athread_join();

  yakl::c::parallel_for("Initialize", yakl::c::Bounds<1>(N), YAKL_LAMBDA(int i) {
    a(i) = i;
    b(i) = i + 1;
    c(i) = 0;
  });

  MulFunctor functor(a, b, c);

  auto start = std::chrono::system_clock::now();
  for (int i = 0; i < N; i ++) {
    c(i) = a(i) * b(i);
  }
  auto end = std::chrono::system_clock::now();
  auto elapesd_CPE = std::chrono::duration<double>(end - start);

 
  // printf("Kernel Finished\n");
  std::cout << "Kernel Finished\n";

  start = std::chrono::system_clock::now();
  
  yakl::c::parallel_for("Compute", yakl::c::Bounds<1>(N), YAKL_LAMBDA(int i) {
    c(i) = a(i) * b(i);
  });
  end = std::chrono::system_clock::now();
  auto elapesd_MPE = std::chrono::duration<double>(end - start);

  yakl::c::parallel_for("Compute_class", yakl::c::Bounds<1>(N), functor);
  // for (int i = 0; i < N; i ++) {
  //   if (c(i) != a(i) * b(i)) {
  //     printf("Error on index %ld\n", i);
  //     break;
  //   }
  // }
  // printf("CPE version costs %lf\nMPE version costs %lf\n", elapesd_CPE.count(), elapesd_MPE.count());
  std::cout << "CPE version costs " << elapesd_CPE.count() << "\nMPE version costs " << elapesd_MPE.count() << std::endl;
}

  yakl::finalize();
  // std::cout << "Program Exiting" << std::endl;
  // SW_BKPT(finalize);
}