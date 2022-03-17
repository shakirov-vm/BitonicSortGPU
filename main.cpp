//-----------------------------------------------------------------------------
//
// Source code for MIPT ILab
// Slides: https://sourceforge.net/projects/cpp-lects-rus/files/cpp-graduate/
// Licensed after GNU GPL v3
//
//-----------------------------------------------------------------------------
//
// Simple vectoradd OpenCL application
//
// clang++ -o vectoradd.exe cl_vectoradd.cc -lOpenCL
//
//-----------------------------------------------------------------------------

#include <sstream>
#include <algorithm>
#include <cassert>
#include <charconv>
#include <chrono>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <vector>

#ifndef CL_HPP_TARGET_OPENCL_VERSION
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#endif

#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_ENABLE_EXCEPTIONS

#include "CL/opencl.hpp"

#ifndef ANALYZE
#define ANALYZE 1
#endif

#define dbgs                                                                   \
  if (!ANALYZE) {                                                              \
  } else                                                                       \
    std::cout

constexpr size_t ARR_SIZE = 64;
constexpr size_t LOCAL_SIZE = 1;

#define STRINGIFY(...) #__VA_ARGS__

struct option_error : public std::runtime_error {
  option_error(const char *s) : std::runtime_error(s) {}
};

// This example have built-in kernel to easy modify, etc

// ---------------------------------- OpenCL ---------------------------------

// OpenCL application encapsulates platform, context and queue
// We can offload vector addition through its public interface

class OclApp {

    cl::Platform P_;
    cl::Context C_;
    cl::CommandQueue Q_;
    std::string K_;

    static cl::Platform select_platform();
    static cl::Context get_gpu_context(cl_platform_id);
    static std::string readFile(const char *);

    using bitonic_t = cl::KernelFunctor<cl::Buffer, size_t>;                 //

public:
    OclApp() : P_(select_platform()), C_(get_gpu_context(P_())), Q_(C_), K_(readFile("./kernel.cl")) {

        cl::string name = P_.getInfo<CL_PLATFORM_NAME>();
        cl::string profile = P_.getInfo<CL_PLATFORM_PROFILE>();
        dbgs << "Selected: " << name << ": " << profile << std::endl;
    }

    // C[i] = A[i] + B[i]
    // Here we shall ask ourselfes: why not template?
    cl::Event bitonic(cl_int *APtr, size_t Sz);                  //
};

std::string OclApp::readFile(const char *Path) {

    std::string Code;
    std::ifstream ShaderFile;
   
    ShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    ShaderFile.open(Path);
  
    std::stringstream ShaderStream;
    ShaderStream << ShaderFile.rdbuf();
  
    ShaderFile.close();
    Code = ShaderStream.str();
  
    return Code;
}

// select first platform with some GPUs
cl::Platform OclApp::select_platform() {

    cl::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
  
    for (auto p : platforms) {
        // note: usage of p() for plain id
        cl_uint numdevices = 0;
        ::clGetDeviceIDs(p(), CL_DEVICE_TYPE_GPU, 0, NULL, &numdevices);
        if (numdevices > 0)
            return cl::Platform(p); // retain?
    }
    throw std::runtime_error("No platform selected");
}

// get context for selected platform
cl::Context OclApp::get_gpu_context(cl_platform_id PId) {
  cl_context_properties properties[] = {
      CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(PId),
      0 // signals end of property list
  };

  return cl::Context(CL_DEVICE_TYPE_GPU, properties);
}

cl::Event OclApp::bitonic(cl_int *APtr, size_t Sz) {

    size_t BufSz = Sz * sizeof(cl_int);

    cl::Buffer A(C_, CL_MEM_READ_WRITE, BufSz);

    cl::copy(Q_, APtr, APtr + Sz, A);

    // try forget context here and happy debugging CL_INVALID_MEM_OBJECT:
    // cl::Program program(vakernel, true /* build immediately */);
    cl::Program program(C_, K_, true /* build immediately */);

    bitonic_t bitonic_simple(program, "bitonic__simple");                         //////

    cl::NDRange GlobalRange(Sz);
    cl::NDRange LocalRange(LOCAL_SIZE);
    cl::EnqueueArgs Args(Q_, GlobalRange, LocalRange);

    cl::Event evt;

    for (int k = 2; k <= ARR_SIZE; k *= 2) {

        for (int j = k / 2; j > 0; j /= 2) {
         
            evt = bitonic_simple(Args, A, k, j);
            evt.wait();
        }
    }

    cl::copy(Q_, A, APtr, APtr + Sz);
    return evt;
}

template <typename It> void rand_init(It start, It end, int low, int up) {
  static std::mt19937_64 mt_source;
  std::uniform_int_distribution<int> dist(low, up);
  for (It cur = start; cur != end; ++cur)
    *cur = dist(mt_source);
}


int main(int argc, char **argv) try {

    std::chrono::high_resolution_clock::time_point TimeStart, TimeFin;
    cl_ulong GPUTimeStart, GPUTimeFin;
    
    long Dur, GDur;
    
    dbgs << "Hello from bitonic" << std::endl;

    OclApp App;

    cl::vector<int> A(ARR_SIZE);
    cl::vector<int> A_copy = A;

    // random initialize -- we just want to excersize and measure
    rand_init(A.begin(), A.end(), 0, 1000);

    // do matrix multiply
    TimeStart = std::chrono::high_resolution_clock::now();

    cl::Event Evt = App.bitonic(A.data(), ARR_SIZE);
    
    TimeFin = std::chrono::high_resolution_clock::now();
    
    Dur = std::chrono::duration_cast<std::chrono::milliseconds>(TimeFin - TimeStart).count();

    std::cout << "GPU wall time measured: " << Dur << " ms" << std::endl;

    GPUTimeStart = Evt.getProfilingInfo<CL_PROFILING_COMMAND_START>();

    GPUTimeFin = Evt.getProfilingInfo<CL_PROFILING_COMMAND_END>();

    GDur = (GPUTimeFin - GPUTimeStart) / 1000000; // ns -> ms

    std::cout << "GPU pure time measured: " << GDur << " ms" << std::endl;

#ifdef VISUALIZE
    std::cout << "--- Matrix ---\n";
    outm(C.data(), Cfg.AX, Cfg.BY);
    std::cout << "--- End Matrix ---\n";
#endif

#if COMPARE_CPU

    
    TimeStart = std::chrono::high_resolution_clock::now();
    
    std::sort(A_copy.begin(), A_copy.end());

    TimeFin = std::chrono::high_resolution_clock::now();

    Dur = std::chrono::duration_cast<std::chrono::milliseconds>(TimeFin - TimeStart).count();
    
    std::cout << "CPU time measured: " << Dur << " ms" << std::endl;

#ifdef VISUALIZE
    std::cout << "--- Matrix ---\n";
    outm(CCPU.data(), Cfg.AX, Cfg.BY);
    std::cout << "--- End Matrix ---\n";
#endif

    for (int i = 0; i < ARR_SIZE; ++i) {
        
        auto lhs = A[i];
        auto rhs = A_copy[i];
        if (lhs != rhs) {
          
            std::cerr << "Error at index " << i << ": " << lhs << " != " << rhs << std::endl;
            return -1;
        }
    }
#endif

  dbgs << "All checks passed" << std::endl;
} catch (cl::BuildError &err) {
  std::cerr << "OCL BUILD ERROR: " << err.err() << ":" << err.what() << std::endl;
  std::cerr << "-- Log --\n";
  for (auto e : err.getBuildLog())
    std::cerr << e.second;
  std::cerr << "-- End log --\n";
  return -1;
} catch (cl::Error &err) {
  std::cerr << "OCL ERROR: " << err.err() << ":" << err.what() << std::endl;
  return -1;
} catch (option_error &err) {
  std::cerr << "INVALID OPTION: " << err.what() << std::endl;
  return -1;
} catch (std::runtime_error &err) {
  std::cerr << "RUNTIME ERROR: " << err.what() << std::endl;
  return -1;
} catch (...) {
  std::cerr << "UNKNOWN ERROR\n";
  return -1;
}