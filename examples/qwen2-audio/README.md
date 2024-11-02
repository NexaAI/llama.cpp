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

## Prepare GGUF 

### 1. proj (mel-filter + whisper-encoder + audio-projector)

1. Extract whisper-large encoder (`audio_tower`), projector and LLM from [`Qwen/Qwen2-Audio-7B-Instruct`](https://huggingface.co/Qwen/Qwen2-Audio-7B-Instruct?show_file_info=model.safetensors.index.json)
```shell
python convert-to-gguf-f16.py
```

2. Extract mel filters from `ggml-large-v3.bin` and add thems to previously extracted `qwen2-audio-instruct.audio_tower-multi_modal_projector.gguf`
```shell
wget --show-progress -O ggml-large-v3.bin https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-large-v3.bin
python add-mel-filters.py
```

### 2. LLM (gemma2)
```shell
python qwen2_surgery.py
```
Then convert to GGML
```shell
cd ../../ # back to root dir
python convert_hf_to_gguf.py examples/qwen2-audio/qwen2
```

### 3 Upload
```
python upload_gguf.py
```

## Run qwen2-audio-cli

```shell
cd llama.cpp.origin

./build/bin/hf-qwen-audio-cli \
    --model examples/qwen2-audio/qwen2/Qwen2-7.8B-F16.gguf \
    --mmproj examples/qwen2-audio/qwen2-audio-instruct.mel-filters-audio_tower-multi_modal_projector.gguf \
    --file examples/qwen2-audio/jfk.wav \
    --prompt "transcribe this audio for me and tell me what it says"
```

## Run python binding

```shell
python nexa_sdk.py \
    --model qwen2/Qwen2-7.8B-F16.gguf \
    --mmproj qwen2-audio-instruct.mel-filters-audio_tower-multi_modal_projector.gguf
```