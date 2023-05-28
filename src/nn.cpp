#ifdef NN
#include "nn.hpp"
#include <ATen/core/TensorBody.h>
#include <c10/core/DeviceType.h>
#include <torch/types.h>
#include <algorithm>
#include <vector>

extern "C" int load_nn(const char* model_path, int32_t cpu)
{
    try 
    {
        _NN_CPU = cpu;
        torch::Device device(cpu == 1 ? torch::kCPU : torch::kCUDA);
        auto module = torch::jit::load(model_path);
        module.to(device);
        _NN_LOADED_MODULE = new std::shared_ptr<torch::jit::Module>(std::make_shared<torch::jit::Module>(module));
    }
    catch (const c10::Error& e) 
    {
        return 1;
    }
    
    return 0;
}

extern "C" void unload_nn()
{
    delete _NN_LOADED_MODULE;
}

extern "C" void forward_nn(float* input, int32_t in_n, float* output, int32_t out_n)
{
    torch::Device device(_NN_CPU == 1 ? torch::kCPU : torch::kCUDA);
    std::shared_ptr<torch::jit::Module> module = (*_NN_LOADED_MODULE);

    auto options = torch::TensorOptions().dtype(torch::kFloat32);
    torch::Tensor t1 = torch::from_blob(
          input,
          {1, in_n},
          options);

    std::vector<torch::jit::IValue> t1_batch;
    t1_batch.push_back(t1.to(device));

    torch::Tensor t2 = module->forward(t1_batch).toTensor().to(torch::kCPU);
    std::copy_n(t2.contiguous().data_ptr<float>(), out_n, output);
}
#endif