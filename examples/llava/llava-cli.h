#ifndef LLAVA_H
#define LLAVA_H

#include <stdbool.h>
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

struct llama_context;
struct llama_model;
struct clip_ctx;
struct llava_image_embed;
struct common_params;
struct common_sampler;

struct llava_context {
    struct clip_ctx       * ctx_clip;
    struct llama_context  * ctx_llama;
    struct llama_model    * model;
};

bool eval_tokens(struct llama_context * ctx_llama,
                 std::vector<int> tokens,  // 注意：原代码中是 std::vector<llama_token>
                 int n_batch,
                 int * n_past);

bool eval_id(struct llama_context * ctx_llama,
             int id,
             int * n_past);

bool eval_string(struct llama_context * ctx_llama,
                 const char* str,
                 int n_batch,
                 int * n_past,
                 bool add_bos);

const char * sample(struct common_sampler * smpl,
                    struct llama_context * ctx_llama,
                    int * n_past);

bool prompt_contains_image(const std::string& prompt);

llava_image_embed * llava_image_embed_make_with_prompt_base64(struct clip_ctx * ctx_clip,
                                                             int n_threads,
                                                             const std::string& prompt);

std::string remove_image_from_prompt(const std::string& prompt, const char * replacement = "");

llama_model   * llava_init(struct common_params * params);
llava_context * llava_init_context(struct common_params * params, llama_model * model);
void            llava_free(struct llava_context * ctx_llava);

void process_prompt(struct llava_context * ctx_llava,
                    struct llava_image_embed * image_embed,
                    common_params * params,
                    const std::string & prompt);

llava_image_embed * load_image(llava_context * ctx_llava,
                               common_params * params,
                               const std::string & fname);
#ifdef __cplusplus
}
#endif

#endif // LLAVA_H
