#include "YAKL.h"
#include <chrono>
#include <iomanip>
#include <ios>
#include <iostream>
#include <istream>
using namespace std;


const int N = 1 << 15;

using yakl::Array;
using yakl::memHost; 
using yakl::styleC;//应该是yakl当中的一些特殊的关键字


typedef Array<int, 1, memHost, styleC> arr1d_host;       //这个arr1d_host可以在CPU上分配内存


using yakl::memDevice; 
typedef Array<int, 1, memDevice, styleC> arr1d_device;   //这个arr1d_device可以在加速器上分配内存


using yakl::c::parallel_for; 
using yakl::c::Bounds;          //YAKL当中和循环相关的一些变量


int main()
{
    printf("hello world\n");
    yakl::init();
    {
        arr1d_host a("a", N); // 替换int a[N];
        arr1d_host b("b", N); // 替换int b[N];
        arr1d_host c("c", N); // 替换int c[N];
        //-------------------------------------------在 CPU 上分配内存完毕

        arr1d_device ad("ad", N);
        arr1d_device bd("bd", N);
        arr1d_device cd("cd", N);
        //-------------------------------------------在 加速器 上分配内存完毕

        //for (int i = 0; i < N; i ++) 
        //{
        //    a[i] = 10;
        //    b[i] = 100;
        //}

        parallel_for("init", Bounds<1>(N), // 在加速器上，初始化相应数组
             YAKL_LAMBDA(int i) {
               ad(i) = 10;
               bd(i) = 100;
             });

        //for (int i = 0; i < N; i ++) {
        //    c[i] = b[i] + a[i];  
        //}

        parallel_for("perform c=a+b", Bounds<1>(N), 
             YAKL_LAMBDA(int i) {
               cd(i) = ad(i) + bd(i); 
             });

        // for (int i = N / 2; i < N - 1; i ++) {
        //     c[i] *= 2;
        // }

        //parallel_for("perform c*=2", Bounds<2>((int)(N / 2), N-1), 
        parallel_for("perform c*=2", Bounds<1>({N/2, N-1}),
             YAKL_LAMBDA(int i) {
               cd(i) *= 2;
             });


    }
    yakl::finalize();

    printf("hello world\n");
    return 0;
}