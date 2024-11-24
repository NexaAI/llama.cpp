#pragma once

#include "llama.h"
#include "common.h"

#include <string>

#ifdef LLAMA_SHARED
#    if defined(_WIN32) && !defined(__MINGW32__)
#        ifdef LLAMA_BUILD
#            define PARALLEL_API __declspec(dllexport)
#        else
#            define PARALLEL_API __declspec(dllimport)
#        endif
#    else
#        define PARALLEL_API __attribute__ ((visibility ("default")))
#    endif
#else
#    define PARALLEL_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct parallel_context_params
{
    const char *model;
    const char *system_prompt;
    int32_t n_parallel;
    int32_t n_gpu_layers;
};

struct parallel_context
{
    llama_context *ctx;
    llama_model *model;

    int32_t n_parallel;
    int32_t n_tokens_system;

    struct llama_batch batch;
};

PARALLEL_API bool parallel_context_params_parse(int argc, char **argv, parallel_context_params &params);

PARALLEL_API parallel_context_params parallel_context_default_params();

PARALLEL_API parallel_context *parallel_init_context(parallel_context_params &params);

PARALLEL_API char **parallel_inference(parallel_context *parallel_ctx, char **prompts);

PARALLEL_API void parallel_free(parallel_context *parallel_ctx);

PARALLEL_API void free_responses(char **responses, int num);

#ifdef __cplusplus
}
#endif