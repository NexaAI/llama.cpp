#ifndef OMNI_AUDIO_H
#define OMNI_AUDIO_H

#include "whisper.h"
#include "llama.h"
#include "grammar-parser.h"
#include "common.h"
#include "common-nexa.h"

#ifdef OMNI_AUDIO_SHARED
#    if defined(_WIN32) && !defined(__MINGW32__)
#        ifdef OMNI_AUDIO_BUILD
#            define OMNI_AUDIO_API __declspec(dllexport)
#        else
#            define OMNI_AUDIO_API __declspec(dllimport)
#        endif
#    else
#        define OMNI_AUDIO_API __attribute__ ((visibility ("default")))
#    endif
#else
#    define OMNI_AUDIO_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *model;
    const char *mmproj;
    const char *file;
    const char *prompt;
    int32_t n_gpu_layers;
} omni_context_params;

typedef struct {
    struct whisper_context *ctx_whisper;
    struct audio_projector *projector;
    struct llama_context *ctx_llama;
    struct llama_model *model;
} omni_context;

typedef struct omni_streaming omni_streaming;

OMNI_AUDIO_API int omni_context_params_parse(int argc, char **argv, omni_context_params *params);

OMNI_AUDIO_API omni_context_params omni_context_default_params(void);

OMNI_AUDIO_API omni_context* omni_init_context(omni_context_params *params);

OMNI_AUDIO_API void omni_free(omni_context *ctx_omni);

OMNI_AUDIO_API const char* omni_process_full(
    omni_context *ctx_omni,
    omni_context_params *params
);

OMNI_AUDIO_API omni_streaming* omni_process_streaming(
    omni_context *ctx_omni,
    omni_context_params *params
);

OMNI_AUDIO_API int32_t omni_sample(omni_streaming *streaming);

OMNI_AUDIO_API const char* omni_get_str(omni_streaming *streaming);

#ifdef __cplusplus
}
#endif

#endif 