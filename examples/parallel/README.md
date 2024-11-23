# llama.cpp/example/parallel

Simplified simulation of serving incoming requests in parallel

```bash
./llama-parallel -m ./models/llama-7b-v2/ggml-model-f16.gguf --gpu-layers 30 -np 4 -ns 4
```

```bash
./llama-parallel-cli -m ./gemma-2-2b-it-GGUF/2b_it_v2.gguf --gpu-layers 27 -np 4
```