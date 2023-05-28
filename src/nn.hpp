#include <torch/script.h>
#include <memory>
#include <utility>

static std::shared_ptr<torch::jit::Module>* _NN_LOADED_MODULE;
static int32_t _NN_CPU;

extern "C" int load_nn(const char* model_path, int32_t cpu);
extern "C" void unload_nn();

extern "C" void forward_nn(float* input, int32_t in_n, float* output, int32_t out_n);