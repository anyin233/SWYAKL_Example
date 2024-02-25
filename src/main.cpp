#include "sw_helper.h"
#include <ios>
#include <iostream>
#include "athread.h"
#include "YAKL.h"
#include <chrono>
#include <iomanip>
#include <istream>

const size_t N = 1 << 12;
using C1Array = yakl::Array<int, 1, yakl::memHost, yakl::styleC>;
using C2Array = yakl::Array<int, 2, yakl::memHost, yakl::styleC>;

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
            << "fake_std::mutex = " << sizeof(yakl::yakl_std_wrapper::mutex) << " fake_std::lock_guard = " << sizeof(yakl::yakl_std_wrapper::lock_guard<yakl::yakl_std_wrapper::mutex>) << std::endl;
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

  std::cout << "Initialize finished\n";

  MulFunctor functor(a, b, c);

  auto start = std::chrono::system_clock::now();
  for (int i = 0; i < N; i ++) {
    c(i) = a(i) * b(i);
  }
  auto end = std::chrono::system_clock::now();
  auto elapesd_MPE = std::chrono::duration<double>(end - start);
  std::cout << "Serial Version Finished\n";
  // printf("Kernel Finished\n");

  start = std::chrono::system_clock::now();
  yakl::c::parallel_for("Compute", yakl::c::Bounds<1>(N), YAKL_LAMBDA(int i) {
    c(i) = a(i) * b(i);
  });
  end = std::chrono::system_clock::now();
  auto elapesd_CPE = std::chrono::duration<double>(end - start);

  std::cout << "LAMBDA Kernel Finished\n";

  start = std::chrono::system_clock::now();
  yakl::c::parallel_for("Compute_class", yakl::c::Bounds<1>(N), functor);
  end = std::chrono::system_clock::now();
  auto elapsed_CPE_Functor = std::chrono::duration<double>(end - start);

  std::cout << "Class Kernel Finished\n";

  C2Array aa("aa", N, N), bb("bb", N, N), cc("cc", N, N);
  yakl::c::parallel_outer("Compute Hierarchical", yakl::c::Bounds<1>(N), YAKL_LAMBDA(int i, yakl::InnerHandler handler) {
    yakl::c::parallel_inner(N, [&] (int j) {
      aa(i, j) = i * N + j;
      bb(i, j) = j * N + j;
      cc(i, j) = 0;
    }, handler);
  }, yakl::LaunchConfig<N>());

  start = std::chrono::system_clock::now();
  std::cout << "Hierarical Kernel Finished\n";
  for (int i = 0; i < N; i ++) {
    for (int j = 0; j < N; j ++) {
      cc(i ,j) = aa(i, j) * bb(i, j);
    }
  }
  end = std::chrono::system_clock::now();
  auto elapsed_2D = std::chrono::duration<double>(end - start);

  start = std::chrono::system_clock::now();
  yakl::c::parallel_for("Compute 2D", yakl::c::Bounds<2>(N, N), YAKL_LAMBDA(int i, int j) {
    cc(i, j) = aa(i, j) * bb(i, j);
  });
  end = std::chrono::system_clock::now();
  auto elapsed_2D_CPE = std::chrono::duration<double>(end - start);

  start = std::chrono::system_clock::now();
  yakl::c::parallel_outer("Compute Hierarchical", yakl::c::Bounds<1>(N), YAKL_LAMBDA(int i, yakl::InnerHandler handler) {
    yakl::c::parallel_inner(N, [&] (int j) {
      cc(i, j) = aa(i, j) * bb(i, j);
    }, handler);
  }, yakl::LaunchConfig<N>());
  end = std::chrono::system_clock::now();
  auto elapsed_2D_CPE_Hier = std::chrono::duration<double>(end - start);

  start = std::chrono::system_clock::now();
  yakl::c::parallel_outer("Compute Hierarchical with LDM", yakl::c::Bounds<1>(N), YAKL_LAMBDA(int i, yakl::InnerHandler handler) {
    int *cc_ldm, *aa_ldm, *bb_ldm;
    aa_ldm = handler.alloc_ldm<int>(N);
    bb_ldm = handler.alloc_ldm<int>(N);
    cc_ldm = handler.alloc_ldm<int>(N);
    unsigned send_reply;
    CRTS_dma_iget(aa_ldm, &aa(i, 0), N * sizeof(int), &send_reply);
    CRTS_dma_iget(bb_ldm, &bb(i, 0), N * sizeof(int), &send_reply);
    CRTS_dma_wait_value(&send_reply, 2);
    yakl::c::parallel_inner(N, [&] (int j) {
      cc_ldm[j] = aa_ldm[j] * bb_ldm[j];
    }, handler);
    CRTS_dma_put(&cc(i, 0), cc_ldm, N * sizeof(int));
  }, yakl::LaunchConfig<N>());
  end = std::chrono::system_clock::now();

  auto elapsed_2D_CPE_Hier_LDM = std::chrono::duration<double>(end - start);

  std::cout << "CPE version costs " << elapesd_CPE.count() << "\nMPE version costs " << elapesd_MPE.count() << std::endl;
  std::cout << "CPE Functor version costs " << elapsed_CPE_Functor.count() << std::endl; 
  std::cout << "2D MPE version costs " << elapsed_2D.count() << std::endl;
  std::cout << "2D CPE version costs " << elapsed_2D_CPE.count() << std::endl;
  std::cout << "2D Hierarchical version costs " << elapsed_2D_CPE_Hier.count() << std::endl;
  std::cout << "2D Hierarchical LDM version costs " << elapsed_2D_CPE_Hier_LDM.count() << std::endl;

  for (int i = 0; i < N; i ++) {
    for (int j = 0; j < N; j ++ ) {
      if (cc(i, j) != aa(i, j) * bb(i, j)){
        std::cout << "2D Hierarchical is wrong\n";
        exit(1);
      }
    }
  }
}

  yakl::finalize();
  // std::cout << "Program Exiting" << std::endl;
  // SW_BKPT(finalize);
}