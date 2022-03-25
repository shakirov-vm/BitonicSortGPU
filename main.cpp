#include <sstream>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

// Compile : g++ main.cpp -lOpenCL

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

//constexpr size_t ARR_SIZE = 4194304;
constexpr size_t ARR_SIZE = 16;
constexpr size_t LOCAL_SIZE = 1;

//long GDurAll = 0;

struct option_error : public std::runtime_error {
    option_error(const char *str) : std::runtime_error(str) {}
};

class OclApp {

    cl::Platform platform_;
    cl::Context context_;
    cl::CommandQueue queue_;
    std::string kernel_code_;

    static cl::Platform select_platform();
    static cl::Context get_gpu_context(cl_platform_id);
    static std::string read_file(const char *path);

    using bitonic_t = cl::KernelFunctor<cl::Buffer, int, int>; 
    using before256_t = cl::KernelFunctor<cl::Buffer>;                    

public:

    OclApp() : platform_(select_platform()), context_(get_gpu_context(platform_())), queue_(context_), kernel_code_(read_file("./kernel.cl")) {

        cl::string name    = platform_.getInfo<CL_PLATFORM_NAME>();
        cl::string profile = platform_.getInfo<CL_PLATFORM_PROFILE>();
        dbgs << "Selected: " << name << ": " << profile << std::endl;
    }

    cl::Event bitonic(cl_int *sequence_ptr, size_t Sz);                  
};

std::string OclApp::read_file(const char *path) {

    std::string Code;
    std::ifstream ShaderFile;
   
    ShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    ShaderFile.open(path);
  
    std::stringstream ShaderStream;
    ShaderStream << ShaderFile.rdbuf();
  
    ShaderFile.close();
    Code = ShaderStream.str();
  
    return Code;
}

cl::Platform OclApp::select_platform() {

    cl::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
  
    for (auto it : platforms) {

        cl_uint numdevices = 0;
        
        ::clGetDeviceIDs(it(), CL_DEVICE_TYPE_GPU, 0, NULL, &numdevices);

        if (numdevices > 0) return cl::Platform(it);
    }

    throw std::runtime_error("No platform selected");
}

cl::Context OclApp::get_gpu_context(cl_platform_id PId) {
    
    cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(PId), 0};
    return cl::Context(CL_DEVICE_TYPE_GPU, properties);
}

cl::Event OclApp::bitonic(cl_int *sequence_ptr, size_t sequence_size) {

    cl::Buffer sequence(context_, CL_MEM_READ_WRITE, sequence_size * sizeof(cl_int));

    cl::copy(queue_, sequence_ptr, sequence_ptr + sequence_size, sequence);

    cl::Program program(context_, kernel_code_, true);
    
    bitonic_t bitonic_simple(program, "bitonic_simple");  
    bitonic_t bitonic_hard(program, "bitonic_hard"); 

    /*kernel_code_ = std::string(read_file("./before256.cl"));
    cl::Program program_1(context_, kernel_code_, true);
    before256_t before256_simple(program_1, "before256");*/

    cl::Event event;
    //cl_ulong GPUTimeStart, GPUTimeFin;

    //cl::NDRange global_range(512);
    //cl::EnqueueArgs args(queue_, global_range, local_range);

    /*event = before256_simple(args, sequence);
    event.wait();*/

    int local_x = 0;

    for (int biton_size = 2; biton_size < ARR_SIZE + 1; biton_size *= 2) {

        if (ARR_SIZE / biton_size >= 8) local_x = 8;
        else local_x = ARR_SIZE / biton_size;

        for (int bucket_size = biton_size; bucket_size > 1; bucket_size /= 2) {

            cl::NDRange global_range(ARR_SIZE / biton_size, biton_size / bucket_size, bucket_size / 2);

            if (local_x > 7) {

                cl::NDRange local_range(local_x, 1, 1);
                cl::EnqueueArgs args(queue_, global_range, local_range);
                event = bitonic_hard(args, sequence, biton_size, bucket_size);
            }
            else {

                cl::NDRange local_range(1, 1, 1);
                cl::EnqueueArgs args(queue_, global_range, local_range);

                event = bitonic_simple(args, sequence, biton_size, bucket_size);
            }
            event.wait();

            //GPUTimeStart = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
            //GPUTimeFin = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();

            //GDurAll += (GPUTimeFin - GPUTimeStart) / 1000000; // ns -> ms
        }
    }

    cl::copy(queue_, sequence, sequence_ptr, sequence_ptr + sequence_size);
    return event;
}

template <typename It> void rand_init(It start, It end, int low, int up) {
    
    static std::mt19937_64 mt_source;
    std::uniform_int_distribution<int> dist(low, up);
    
    for (It cur = start; cur != end; ++cur) *cur = dist(mt_source);
}


int main(int argc, char **argv) try {

    std::chrono::high_resolution_clock::time_point TimeStart, TimeFin;
    cl_ulong GPUTimeStart, GPUTimeFin;
    
    long Dur, GDur;
    
    dbgs << "Hello from bitonic" << std::endl;

    OclApp app;

    cl::vector<int> sequence(ARR_SIZE);
    
#ifdef FROM_FILE
    
    std::ifstream in_num("./test.txt");
    //std::ifstream in_num("./_512_.txt");
    //std::ifstream in_num("./_32_.txt");
    

    for(int i = 0; i < ARR_SIZE; i++) {
        in_num >> sequence[i];
    }

#endif

#ifndef FROM_FILE
    rand_init(sequence.begin(), sequence.end(), 0, 100);
#endif

    cl::vector<int> sequence_copy = sequence;
    
    TimeStart = std::chrono::high_resolution_clock::now();
    cl::Event event = app.bitonic(sequence.data(), ARR_SIZE);
    TimeFin = std::chrono::high_resolution_clock::now();
    
    Dur = std::chrono::duration_cast<std::chrono::milliseconds>(TimeFin - TimeStart).count();

    std::cout << "GPU wall time measured: " << Dur << " ms" << std::endl;

    /*GPUTimeStart = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
    GPUTimeFin = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();

    GDur = (GPUTimeFin - GPUTimeStart) / 1000000; // ns -> ms                       Can we count like this?
                                                                                    We have so much events...
    std::cout << "GPU pure time measured: " << GDur << " ms" << std::endl;*/

    //std::cout << "GPU pure time measured: " << GDurAll << " ms" << std::endl;

#ifdef PRINT_SORT

    for (int i = 0; i < ARR_SIZE; i++) {
        std::cout << sequence[i] << " ";
    }
    std::cout << std::endl;

#endif
    
    TimeStart = std::chrono::high_resolution_clock::now();
    std::sort(sequence_copy.begin(), sequence_copy.end());
    TimeFin = std::chrono::high_resolution_clock::now();

    Dur = std::chrono::duration_cast<std::chrono::milliseconds>(TimeFin - TimeStart).count();
    
    std::cout << "CPU time measured: " << Dur << " ms" << std::endl;

    for (int i = 0; i < ARR_SIZE; ++i) {
        
        auto lhs = sequence[i];
        auto rhs = sequence_copy[i];
        if (lhs != rhs) {
          
            std::cerr << "Error at index " << i << ": " << lhs << " != " << rhs << std::endl;
            return -1;
        }
    }

    dbgs << "All checks passed" << std::endl;
} 
catch (cl::BuildError &err) {

    std::cerr << "OCL BUILD ERROR: " << err.err() << ":" << err.what() << std::endl;
    std::cerr << "-- Log --\n";
    for (auto e : err.getBuildLog())
        std::cerr << e.second;
    std::cerr << "-- End log --\n";
    return -1;
} 
catch (cl::Error &err) {

    std::cerr << "OCL ERROR: " << err.err() << ":" << err.what() << std::endl;
    return -1;
} 
catch (option_error &err) {

    std::cerr << "INVALID OPTION: " << err.what() << std::endl;
    return -1;
} 
catch (std::runtime_error &err) {

    std::cerr << "RUNTIME ERROR: " << err.what() << std::endl;
    return -1;
} 
catch (...) {

    std::cerr << "UNKNOWN ERROR\n";
    return -1;
}