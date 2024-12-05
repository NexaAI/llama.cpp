#include "parallel-wrapper.h"
#include "common.h"
#include "llama.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <ctime>

// trim whitespace from the beginning and end of a string
static std::string trim(const std::string &str)
{
    size_t start = 0;
    size_t end = str.size();

    while (start < end && isspace(str[start]))
    {
        start += 1;
    }

    while (end > start && isspace(str[end - 1]))
    {
        end -= 1;
    }

    return str.substr(start, end - start);
}

struct client
{
    ~client()
    {
        if (ctx_sampling)
        {
            llama_sampling_free(ctx_sampling);
        }
    }

    int32_t id = 0;

    llama_seq_id seq_id = -1;

    llama_token sampled;

    int64_t t_start_prompt;
    int64_t t_start_gen;

    int32_t n_prompt = 0;
    int32_t n_decoded = 0;
    int32_t i_batch = -1;

    std::string input;
    std::string prompt;
    std::string response;

    struct llama_sampling_context *ctx_sampling = nullptr;
};

static void print_date_time()
{
    std::time_t current_time = std::time(nullptr);
    std::tm *local_time = std::localtime(&current_time);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);

    printf("\n\033[35mrun parameters as at %s\033[0m\n", buffer);
}

// Define a split string function to ...
static std::vector<std::string> split_string(const std::string &input, char delimiter)
{
    std::vector<std::string> tokens;
    std::istringstream stream(input);
    std::string token;
    while (std::getline(stream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

static void parallel_print_usage(int, char **argv)
{
    LOG("\n example usage:\n");
    LOG("\n     %s --model <parallel/ggml-model.gguf> [--system-prompt <\"system prompt\">] --n-parallel <n>\n", argv[0]);
}

bool parallel_context_params_parse(int argc, char **argv, parallel_context_params &params)
{
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg[0] != '-')
        {
            continue;
        }

        if (arg == "-h" || arg == "--help")
        {
            parallel_print_usage(argc, argv);
            exit(0);
        }
        if (arg == "--system-prompt")
        {
            params.system_prompt = argv[++i];
        }
        else if (arg == "-m" || arg == "--model")
        {
            params.model = argv[++i];
        }
        else if (arg == "--n-ctx")
        {
            params.n_ctx = std::stoi(argv[++i]);
        }
        else if (arg == "-ngl" || arg == "--gpu-layers" || arg == "--n-gpu-layers")
        {
            params.n_gpu_layers = std::stoi(argv[++i]);
        }
        else if (arg == "-np" || arg == "--n-parallel")
        {
            params.n_parallel = std::stoi(argv[++i]);
        }
        else
        {
            fprintf(stderr, "error: unknown argument: %s\n", arg.c_str());
            parallel_print_usage(argc, argv);
            exit(0);
        }
    }

    return true;
}

parallel_context_params parallel_context_default_params()
{
    parallel_context_params params;
    params.model = "";
    params.system_prompt = "";
    params.n_ctx = 8192;
    params.n_parallel = 1;
    params.n_gpu_layers = -1;
    return params;
}

parallel_context *parallel_init_context(parallel_context_params &params)
{
    gpt_params gpt_params;
    gpt_params.model = params.model;
    gpt_params.n_ctx = params.n_ctx;
    gpt_params.n_sequences = params.n_parallel;
    gpt_params.n_parallel = params.n_parallel + 1;
    gpt_params.n_gpu_layers = params.n_gpu_layers;

    llama_init_result llama_init = llama_init_from_gpt_params(gpt_params);
    llama_context *ctx = llama_init.context;
    llama_model *model = llama_init.model;

    parallel_context *parallel_ctx = new parallel_context();
    parallel_ctx->ctx = ctx;
    parallel_ctx->model = model;
    parallel_ctx->n_parallel = params.n_parallel;

    std::vector<llama_token> tokens_system;
    tokens_system = ::llama_tokenize(ctx, params.system_prompt, true);
    parallel_ctx->n_tokens_system = tokens_system.size();
    LOG_TEE("%s: n_tokens_system: %d\n", __func__, parallel_ctx->n_tokens_system);

    const int n_ctx = llama_n_ctx(ctx);
    // the max batch size is as large as the context to handle cases where we get very long input prompt from multiple
    // users. regardless of the size, the main loop will chunk the batch into a maximum of params.n_batch tokens at a time
    llama_batch batch = llama_batch_init(n_ctx, 0, 1);
    parallel_ctx->batch = batch;

    {
        LOG_TEE("%s: Evaluating the system prompt ...\n", __func__);

        for (int32_t i = 0; i < parallel_ctx->n_tokens_system; ++i)
        {
            llama_batch_add(batch, tokens_system[i], i, {0}, false);
        }

        if (llama_decode(ctx, batch) != 0)
        {
            LOG_TEE("%s: llama_decode() failed\n", __func__);
            return {};
        }

        // assign the system KV cache to all parallel sequences
        for (int32_t i = 1; i <= parallel_ctx->n_parallel; ++i)
        {
            llama_kv_cache_seq_cp(ctx, 0, i, -1, -1);
        }

        LOG_TEE("\n");
    }

    return parallel_ctx;
}

char **parallel_inference(parallel_context *parallel_ctx, char **prompts)
{
    llama_context *ctx = parallel_ctx->ctx;
    llama_model *model = parallel_ctx->model;
    const int32_t n_tokens_system = parallel_ctx->n_tokens_system;
    const int32_t n_clients = parallel_ctx->n_parallel;
    const int32_t n_seq = parallel_ctx->n_parallel;

    std::vector<std::string> responses;
    responses.resize(n_clients);

    gpt_params params;
    const bool cont_batching = params.cont_batching;
    const bool dump_kv_cache = params.dump_kv_cache;

    llama_sampling_params sparams;

    std::vector<client> clients(n_clients);
    for (size_t i = 0; i < clients.size(); ++i)
    {
        auto &client = clients[i];
        client.id = i;
        client.ctx_sampling = llama_sampling_init(sparams);
    }

    llama_seq_id g_seq_id = 0;
    struct llama_kv_cache_view kvc_view = llama_kv_cache_view_init(ctx, parallel_ctx->n_parallel);

    llama_batch &batch = parallel_ctx->batch;

    int32_t n_total_prompt = 0;
    int32_t n_total_gen    = 0;
    int32_t n_cache_miss = 0;

    const auto t_main_start = ggml_time_us();

    while (true)
    {
        if (dump_kv_cache)
        {
            llama_kv_cache_view_update(ctx, &kvc_view);
            llama_kv_cache_dump_view_seqs(kvc_view, 40);
        }

        llama_batch_clear(batch);

        // decode any currently ongoing sequences
        for (auto &client : clients)
        {
            if (client.seq_id == -1)
            {
                continue;
            }

            client.i_batch = batch.n_tokens;

            llama_batch_add(batch, client.sampled, n_tokens_system + client.n_prompt + client.n_decoded, {client.id + 1}, true);

            client.n_decoded += 1;
        }

        if (batch.n_tokens == 0)
        {
            // all sequences have ended - clear the entire KV cache
            for (int i = 1; i <= n_clients; ++i)
            {
                llama_kv_cache_seq_rm(ctx, i, -1, -1);
                // but keep the system prompt
                llama_kv_cache_seq_cp(ctx, 0, i, -1, -1);
            }

            LOG_TEE("%s: clearing the KV cache\n", __func__);
        }

        // insert new sequences for decoding
        if (cont_batching || batch.n_tokens == 0)
        {
            for (auto &client : clients)
            {
                if (client.seq_id == -1 && g_seq_id < n_seq)
                {
                    client.seq_id = g_seq_id;

                    client.t_start_prompt = ggml_time_us();
                    client.t_start_gen = 0;

                    client.input = prompts[g_seq_id];
                    client.prompt = client.input + "\nAssistant:";
                    client.response = "";

                    llama_sampling_reset(client.ctx_sampling);

                    // do not prepend BOS because we have a system prompt!
                    std::vector<llama_token> tokens_prompt;
                    tokens_prompt = ::llama_tokenize(ctx, client.prompt, false);

                    for (size_t i = 0; i < tokens_prompt.size(); ++i)
                    {
                        llama_batch_add(batch, tokens_prompt[i], i + n_tokens_system, {client.id + 1}, false);
                    }

                    // extract the logits only for the last token
                    if (batch.n_tokens > 0)
                    {
                        batch.logits[batch.n_tokens - 1] = true;
                    }

                    client.n_prompt = tokens_prompt.size();
                    client.n_decoded = 0;
                    client.i_batch = batch.n_tokens - 1;

                    LOG_TEE("\033[31mClient %3d, seq %4d, started decoding ...\033[0m\n", client.id, client.seq_id);

                    g_seq_id += 1;

                    // insert new requests one-by-one
                    // if (cont_batching) {
                    //    break;
                    //}
                }
            }
        }

        if (batch.n_tokens == 0)
        {
            break;
        }

        // process in chunks of params.n_batch
        int32_t n_batch = params.n_batch;

        for (int32_t i = 0; i < (int32_t)batch.n_tokens; i += n_batch)
        {
            // experiment: process in powers of 2
            // if (i + n_batch > (int32_t) batch.n_tokens && n_batch > 32) {
            //    n_batch /= 2;
            //    i -= n_batch;
            //    continue;
            //}

            const int32_t n_tokens = std::min(n_batch, (int32_t)(batch.n_tokens - i));

            llama_batch batch_view = {
                n_tokens,
                batch.token + i,
                nullptr,
                batch.pos + i,
                batch.n_seq_id + i,
                batch.seq_id + i,
                batch.logits + i,
                0, 0, 0, // unused
            };

            const int ret = llama_decode(ctx, batch_view);
            if (ret != 0)
            {
                if (n_batch == 1 || ret < 0)
                {
                    // if you get here, it means the KV cache is full - try increasing it via the context size
                    LOG_TEE("%s : failed to decode the batch, n_batch = %d, ret = %d\n", __func__, n_batch, ret);
                    return {};
                }

                LOG("%s : failed to decode the batch, retrying with n_batch = %d\n", __func__, n_batch / 2);

                n_cache_miss += 1;

                // retry with half the batch size to try to find a free slot in the KV cache
                n_batch /= 2;
                i -= n_batch;

                continue;
            }

            LOG("%s : decoded batch of %d tokens\n", __func__, n_tokens);

            for (auto &client : clients)
            {
                if (client.i_batch < (int)i || client.i_batch >= (int)(i + n_tokens))
                {
                    continue;
                }

                // printf("client %d, seq %d, token %d, pos %d, batch %d\n",
                //         client.id, client.seq_id, client.sampled, client.n_decoded, client.i_batch);

                const llama_token id = llama_sampling_sample(client.ctx_sampling, ctx, NULL, client.i_batch - i);

                llama_sampling_accept(client.ctx_sampling, ctx, id, true);

                if (client.n_decoded == 1)
                {
                    // start measuring generation time after the first token to make sure all concurrent clients
                    // have their prompt already processed
                    client.t_start_gen = ggml_time_us();
                }

                const std::string token_str = llama_token_to_piece(ctx, id);

                client.response += token_str;
                client.sampled = id;

                // printf("client %d, seq %d, token %d, pos %d, batch %d: %s\n",
                //         client.id, client.seq_id, id, client.n_decoded, client.i_batch, token_str.c_str());

                if (client.n_decoded > 2 &&
                    (llama_token_is_eog(model, id) ||
                     (params.n_predict > 0 && client.n_decoded + client.n_prompt >= params.n_predict) ||
                     client.response.find("User:") != std::string::npos ||
                     client.response.find('\n') != std::string::npos))
                {
                    // basic reverse prompt
                    const size_t pos = client.response.find("User:");
                    if (pos != std::string::npos)
                    {
                        client.response = client.response.substr(0, pos);
                    }

                    // delete only the generated part of the sequence, i.e. keep the system prompt in the cache
                    llama_kv_cache_seq_rm(ctx, client.id + 1, -1, -1);
                    llama_kv_cache_seq_cp(ctx, 0, client.id + 1, -1, -1);

                    const auto t_main_end = ggml_time_us();

                    responses[client.seq_id] = client.response;

                    LOG_TEE("\033[31mClient %3d, seq %3d/%3d, prompt %4d t, response %4d t, time %5.2f s, speed %5.2f t/s, cache miss %d \033[0m \nInput:    %s\n\033[35mResponse: %s\033[0m\n\n",
                            client.id, client.seq_id, n_seq, client.n_prompt, client.n_decoded,
                            (t_main_end - client.t_start_prompt) / 1e6,
                            (double)(client.n_prompt + client.n_decoded) / (t_main_end - client.t_start_prompt) * 1e6,
                            n_cache_miss,
                            ::trim(client.input).c_str(),
                            ::trim(client.response).c_str());

                    n_total_prompt += client.n_prompt;
                    n_total_gen += client.n_decoded;

                    client.seq_id = -1;
                }

                client.i_batch = -1;
            }
        }
    }

    const auto t_main_end = ggml_time_us();

    LOG_TEE("Total prompt tokens: %6d, speed: %5.2f t/s\n", n_total_prompt, (double) (n_total_prompt              ) / (t_main_end - t_main_start) * 1e6);
    LOG_TEE("Total gen tokens:    %6d, speed: %5.2f t/s\n", n_total_gen,    (double) (n_total_gen                 ) / (t_main_end - t_main_start) * 1e6);
    LOG_TEE("Total speed (AVG):   %6s  speed: %5.2f t/s\n", "",             (double) (n_total_prompt + n_total_gen) / (t_main_end - t_main_start) * 1e6);
    LOG_TEE("Cache misses:        %6d\n\n", n_cache_miss);

    char **c_responses = new char *[responses.size()];
    for (size_t i = 0; i < responses.size(); ++i)
    {
        c_responses[i] = strdup(responses[i].c_str());
    }

    return c_responses;
}

void parallel_free(parallel_context *parallel_ctx)
{
    llama_batch_free(parallel_ctx->batch);

    llama_free(parallel_ctx->ctx);
    llama_free_model(parallel_ctx->model);

    llama_backend_free();
}

void free_responses(char **responses, int num)
{
    for (int i = 0; i < num; ++i)
    {
        delete[] responses[i];
    }
    delete[] responses;
}