# qwen2-audio

## Build this example

Build the whole project from repo root dir

```shell
cd llama.cpp.origin
mkdir build && cd build && cmake -DNEXA_DEBUG=ON ..
make -j32
```

Or with CUDA backend support
```shell
mkdir build && cd build && cmake -DGGML_CUDA=ON ..
make -j32
```

## Run nexa-qwen2-audio-cli

```shell
cd llama.cpp.origin

./build/bin/nexa-qwen2-audio-cli \
    --model examples/qwen2-audio/qwen2/Qwen2-7.8B-F16.gguf \
    --mmproj examples/qwen2-audio/qwen2-audio-instruct.mel-filters-audio_tower-multi_modal_projector.gguf \
    --file examples/qwen2-audio/jfk.wav \
    --prompt "transcribe this audio for me and tell me what it says"
```

## Run nexa-omni-cli
```shell
cd llama.cpp.origin

./build/bin/nexa-omni-cli \
    --model examples/nano-omni-audio/gemma2-2b.gguf \
    --mmproj examples/nano-omni-audio/nano-omni-instruct.mel-filters-audio_tower-multi_modal_projector.gguf \
    --file examples/nano-omni-audio/jfk.wav \
    --prompt "this conversation talks about"
```

## Run Python bindings
For qwen2-audio
```shell
python nexa_audio_inference.py \
    --model Qwen2-7.8B-F16.gguf \
    --mmproj qwen2-audio-instruct.mel-filters-audio_tower-multi_modal_projector.gguf \
    --n_gpu_layers -1
```
For nano-omni-audio
```shell
python nexa_audio_inference.py \
    --model gemma2-2b.gguf \
    --mmproj nano-omni-instruct.mel-filters-audio_tower-multi_modal_projector.gguf \
    --n_gpu_layers -1
```