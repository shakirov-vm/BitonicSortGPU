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
#include <climits>

#include "Bitonic.h"

#define LOCAL_SIZE 1

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

//error with work_group_size?
cl::Event OclApp::bitonic(cl_int *sequence_ptr, size_t sequence_size) {

    int new_sequence_size = static_cast<int>(sequence_size); // Whatever this realization of algorithm don't work, if 
                                                             // you have 2GB GPU memory and want sort 8GB array 

    if ((sequence_size & (sequence_size - 1)) != 0) {

        for (int i = 1; i < 64; i <<= 1) new_sequence_size |= (new_sequence_size >> i);
        new_sequence_size++;
    }

    dbgs << "New is " << new_sequence_size << std::endl;
    dbgs << "work_group_size is " << settings.work_group_size << std::endl;
    
    std::vector<int> full_sequence(new_sequence_size, INT_MAX);
    std::copy(sequence_ptr, sequence_ptr + sequence_size, full_sequence.begin());

    cl::Buffer sequence(context_, CL_MEM_READ_WRITE, new_sequence_size * sizeof(cl_int));
    cl::copy(queue_, full_sequence.data(), full_sequence.data() + new_sequence_size, sequence);

    cl::Program program(context_, kernel_code_, true);

    bitonic_t small_biton(program, "small_biton"); 
    bitonic_t big_biton(program, "big_biton"); 
    bitonic_t big_bucket(program, "big_bucket"); 

    cl::Event event;

    std::chrono::high_resolution_clock::time_point TimeStart = std::chrono::high_resolution_clock::now();

    for (int biton_size = 2; biton_size <= new_sequence_size; biton_size *= 2) {

        for (int bucket_size = biton_size; bucket_size >= 2; bucket_size /= 2) {

            cl::NDRange global_range(new_sequence_size / biton_size, biton_size / bucket_size, bucket_size / 2);
                
            if (bucket_size >= settings.work_group_size * 2) {

                cl::NDRange local_range(1, 1, settings.work_group_size);
                cl::EnqueueArgs args(queue_, global_range, local_range);
            
                event = big_bucket(args, sequence, biton_size, bucket_size);
            }
            else if (biton_size > settings.work_group_size * 2) {

                cl::NDRange local_range(1 , 2 * settings.work_group_size / bucket_size, bucket_size / 2);
                cl::EnqueueArgs args(queue_, global_range, local_range);
            
                event = big_biton(args, sequence, biton_size, bucket_size);
            }
            else if (biton_size <= settings.work_group_size * 2) {              

                cl::NDRange local_range(2 * settings.work_group_size / biton_size, biton_size / bucket_size, bucket_size / 2);
                cl::EnqueueArgs args(queue_, global_range, local_range);
            
                event = small_biton(args, sequence, biton_size, bucket_size);
            }
            event.wait(); //Is that need?
        }
    }

    std::chrono::high_resolution_clock::time_point TimeFin = std::chrono::high_resolution_clock::now();

    long Dur = std::chrono::duration_cast<std::chrono::milliseconds>(TimeFin - TimeStart).count();

    dbgs << "GPU events time measured: " << Dur << " ms" << std::endl;

    cl::copy(queue_, sequence, sequence_ptr, sequence_ptr + sequence_size);
    return event;
}

/*
cl::Event OclApp::bitonic(cl_int *sequence_ptr, size_t sequence_size) {

    int new_sequence_size = static_cast<int>(sequence_size); // Whatever this realization of algorithm don't work, if 
                                                             // you have 2GB GPU memory and want sort 8GB array 

    if ((sequence_size & (sequence_size - 1)) != 0) {

        for (int i = 1; i < 64; i <<= 1) new_sequence_size |= (new_sequence_size >> i);
        new_sequence_size++;
    }

    dbgs << "New is " << new_sequence_size << std::endl;
    dbgs << "work_group_size is " << settings.work_group_size << std::endl;
    
    std::vector<int> full_sequence(new_sequence_size, INT_MAX);
    std::copy(sequence_ptr, sequence_ptr + sequence_size, full_sequence.begin());

    cl::Buffer sequence(context_, CL_MEM_READ_WRITE, new_sequence_size * sizeof(cl_int));
    cl::copy(queue_, full_sequence.data(), full_sequence.data() + new_sequence_size, sequence);

    cl::Program program(context_, kernel_code_, true);
    
    bitonic_t bitonic_simple(program, "bitonic_simple");  

    cl::Event event;

    std::chrono::high_resolution_clock::time_point TimeStart = std::chrono::high_resolution_clock::now();

    for (int biton_size = 2; biton_size < new_sequence_size + 1; biton_size *= 2) {

        for (int bucket_size = biton_size; bucket_size > 1; bucket_size /= 2) {
         
            cl::NDRange global_range(new_sequence_size / biton_size, biton_size / bucket_size, bucket_size / 2);
            cl::NDRange local_range(LOCAL_SIZE, LOCAL_SIZE, LOCAL_SIZE);

            cl::EnqueueArgs args(queue_, global_range, local_range);

            event = bitonic_simple(args, sequence, biton_size, bucket_size);
            event.wait();
        }
    }

    cl::copy(queue_, sequence, sequence_ptr, sequence_ptr + sequence_size);
    return event;
}
*/