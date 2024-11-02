#include "audio-projector.h"
#include "common-nexa.h"

#include "ggml.h"

#include <string>
#include <vector>

int main(int argc, char **argv)
{
    ggml_time_init(); // Initialize timing

    const int64_t t_main_start_us = ggml_time_us(); // Start timing

    std::string projector_path = "./models/nano-omni-instruct.mel-filters-audio_tower-multi_modal_projector-f16.gguf";
    if (argc >= 2)
    {
        projector_path = argv[1];
    }

    audio_projector model;
    if (!model.load_from_gguf(projector_path)) // Load the projector model
    {
        fprintf(stderr, "Failed to load model.\n");
        return -1;
    }

    // Prepare input data
    std::vector<float> input_data(1024 * 750, 1.0f);

    // Compute the result using the backend
    struct ggml_tensor *result = audio_projector_inference(model, input_data);

    // print_ggml_tensor("Result tensor", result, true, 5);
    print_ggml_tensor_shape("Result tensor", result);

    // Report timing
    const int64_t t_main_end_us = ggml_time_us();
    printf("%s: total time = %8.2f ms\n", __func__, (t_main_end_us - t_main_start_us) / 1000.0f);

    return 0;
}