#include "omni-vlm-wrapper.h"

int main(int argc, char ** argv) {
    const char* llm_model = "/Users/liwei/huggingface/nano-llm/Nano-Llm-494M-F16.gguf";
    const char* mmproj_model = "/Users/liwei/huggingface/nano-vlm-instruct/mmproj-omni-vlm-f16.gguf";
    const char* image_path = "/Users/liwei/repo/llama-cpp-experiments/huggingface/cat.png";
    const char* prompt = "<|im_start|>system\nYou are Nano-Omni-VLM, created by Nexa AI. You are a helpful assistant.<|im_end|>\n<|im_start|>user\nDescribe this image for me\n<|vision_start|><|image_pad|><|vision_end|><|im_end|>";

    omnivlm_init(llm_model, mmproj_model);
    omnivlm_inference(prompt, image_path);
    omnivlm_inference(prompt, image_path);
    omnivlm_free();

    return 0;
}
