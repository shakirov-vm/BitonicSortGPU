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

#include "./BitonicLib/Bitonic.h"

// Compile : g++ main.cpp -lOpenCL -DFROM_FILE -DPRINT_SORT -O2
/*
#ifndef MODE
#define MODE "default"
#endif

#if (strcmp(MODE, "default_mode"))
#define PATH "BitonicLib/Kernels/fast.cl"

#elif (std::strcmp(MODE, "fast"))
#define PATH "BitonicLib/Kernels/fast.cl"

#elif (std::strcmp(MODE, "simple"))
#define PATH "BitonicLib/Kernels/simple.cl"

#endif
*/

#define PATH "BitonicLib/Kernels/fast.cl"

#define MAX_RAND_INIT 10000000

template <typename It> void rand_init(It start, It end, int low, int up) {
    
    static std::mt19937_64 mt_source;
    std::uniform_int_distribution<int> dist(low, up);
    
    for (It cur = start; cur != end; ++cur) *cur = dist(mt_source);
}


int main(int argc, char **argv) try {
/*
#if (std::strcmp(MODE, "default_mode"))
    std::cout << "Default mode choosen" << std::endl;
#endif
*/
    if (argc != 3) {
        std::cout << "First arg is arr_size, second is work_group_size, your num of params is invalid" << std::endl;
        return -1;
    }

    size_t arr_size = std::atoi(argv[1]);
    cl_int work_group_size = static_cast<int>(std::atoi(argv[2]));

    if ((work_group_size & (work_group_size - 1)) != 0) {
        std::cout << "work_group_size must be a power of 2" << std::endl;
        return -1;
    }

    std::chrono::high_resolution_clock::time_point TimeStart, TimeFin;
    
    dbgs << "Hello from bitonic" << std::endl;

    OclApp app(PATH, work_group_size);

    cl::vector<int> sequence(arr_size);
    
#ifdef FROM_FILE
    
    //std::ifstream in_num("./Tests/test.txt");
    //std::ifstream in_num("./Tests/_512_.txt");
    std::ifstream in_num("./Tests/_32_.txt");
    
    arr_size = 32; // ?

    for(int i = 0; i < arr_size; i++) {
        in_num >> sequence[i];
    }

#endif

#ifndef FROM_FILE
    rand_init(sequence.begin(), sequence.end(), 0, MAX_RAND_INIT);
#endif

    cl::vector<int> sequence_copy = sequence;
    
    TimeStart = std::chrono::high_resolution_clock::now();
    cl::Event event = app.bitonic(sequence.data(), arr_size);
    TimeFin = std::chrono::high_resolution_clock::now();
    
    long Dur = std::chrono::duration_cast<std::chrono::milliseconds>(TimeFin - TimeStart).count();

    std::cout << "GPU wall time measured: " << Dur << " ms" << std::endl;

#ifdef PRINT_SORT

    for (int i = 0; i < arr_size; i++) {
        std::cout << sequence[i] << " ";
    }
    std::cout << std::endl;

#endif
    
    TimeStart = std::chrono::high_resolution_clock::now();
    std::sort(sequence_copy.begin(), sequence_copy.end());
    TimeFin = std::chrono::high_resolution_clock::now();

    Dur = std::chrono::duration_cast<std::chrono::milliseconds>(TimeFin - TimeStart).count();
    
    std::cout << "CPU time measured: " << Dur << " ms" << std::endl;

    for (size_t i = 0; i < arr_size; ++i) {
        
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