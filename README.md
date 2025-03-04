# llama.cpp

This repo is cloned from llama.cpp [commit 06c2b1561d8b882bc018554591f8c35eb04ad30e](https://github.com/ggml-org/llama.cpp/tree/06c2b1561d8b882bc018554591f8c35eb04ad30e). It is compatible with llama-cpp-python [commit 710e19a81284e5af0d5db93cef7a9063b3e8534f](https://github.com/abetlen/llama-cpp-python/tree/710e19a81284e5af0d5db93cef7a9063b3e8534f)

## Customize quantization group size at compilation (CPU inference only)

The only thing that is different is to add -DQK4_0 flag when cmake.

```bash
cmake -B build_cpu_g128 -DQK4_0=128
cmake --build build_cpu_g128
```

To quantize the model with the customized group size, run

```bash
./build_cpu_g128/bin/llama-quantize <model_path.gguf> <quantization_type>
```

To run the quantized model, run

```bash
./build_cpu_g128/bin/llama-cli -m <quantized_model_path.gguf>
```

### Note:

You should make sure that the model you run is quantized to the same group size as the one you compile with.
Or you'll receive a runtime error when loading the model.