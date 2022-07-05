#pragma once

#include <string>

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

struct option_error : public std::runtime_error {
    option_error(const char *str) : std::runtime_error(str) {}
};

struct settings_OL {

	int work_group_size;

	settings_OL(int work_group_size_ = 8) : work_group_size(work_group_size_) {}
};

class OclApp {

	struct settings_OL settings;
    cl::Platform platform_;
    cl::Context context_;
    cl::CommandQueue queue_;
    std::string kernel_code_;

    static cl::Platform select_platform();
    static cl::Context get_gpu_context(cl_platform_id);
    static std::string read_file(const char *path);

    using bitonic_t = cl::KernelFunctor<cl::Buffer, int, int>;

    void set_static_params() {

        std::string num = std::to_string(settings.work_group_size);
        auto position = kernel_code_.find("#define WORK_GROUP_SIZE"); // what's type?
    // "#define WORK_GROUP_SIZE %" <-- we want to replace it to num, this position in string is 24
        kernel_code_.replace(position + 24, 10, num);
    }

public:

    OclApp(const char* path, cl_int work_group_size_ = 8) : settings(work_group_size_), platform_(select_platform()),
    													    context_(get_gpu_context(platform_())), queue_(context_),
    													    kernel_code_(read_file(path)) {

        cl::string name    = platform_.getInfo<CL_PLATFORM_NAME>();
        cl::string profile = platform_.getInfo<CL_PLATFORM_PROFILE>();
        dbgs << "Selected: " << name << ": " << profile << std::endl;

        set_static_params();
    }

    cl::Event bitonic(cl_int *sequence_ptr, size_t sequence_size);                  
};