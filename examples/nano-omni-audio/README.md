# nexa-audio-omni

## Build this example

Build the whole project from repo root dir

```shell
cd llama.cpp.origin
mkdir build && cd build && cmake ..
make -j32
```

Or with CUDA backend support
```shell
mkdir build && cd build && cmake -DGGML_CUDA=ON ..
make -j32
```

Or with Metal backend support
```shell
mkdir build && cd build && cmake -DGGML_METAL=ON -DBUILD_SHARED_LIBS=Off ..
make -j32
```

The dynamic lib is generated at 
```
build/examples/nano-omni-audio/libhf-omni-audio-cli_shared.so
```

## Prepare GGUF 

### 1. proj (mel-filter + whisper-encoder + audio-projector)

1. Extract whisper-medium encoder (`audio_tower`), projector and LLM from [`nexa-collaboration/nano-omini-instruct`](https://huggingface.co/nexa-collaboration/nano-omini-instruct/tree/main?show_file_info=model.safetensors.index.json)
```shell
python convert-to-gguf-f16.py
```

2. Extract mel filters from `ggml-medium.bin` and add thems to previously extracted `nano-omni-audio-encoder-f16.gguf`
```shell
wget --show-progress -O ggml-medium.bin https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-medium.bin
python add-mel-filters.py
```

### 2. LLM (gemma2)
```shell
python gemma2_surgery.py
```
Then convert to GGML
```
cd ../../../ # back to root dir
python convert_hf_to_gguf.py examples/nano-omni-audio/gemma2
```

### 3 Upload
```
python upload_gguf.py
```

## Run nexa-audio-omni
```shell
./build/bin/omni-cli \
    --model examples/nano-omni-audio/gemma2-2b.gguf \
    --mmproj examples/nano-omni-audio/nano-omni-instruct.mel-filters-audio_tower-multi_modal_projector.gguf \
    --file examples/nano-omni-audio/jfk.wav \
    --prompt "this conversation talks about"
```

## Run python binding
```
python nexa_sdk.py \
    --model gemma2-2b.gguf \
    --mmproj nano-omni-instruct.mel-filters-audio_tower-multi_modal_projector.gguf \
    --n_gpu_layers -1
```