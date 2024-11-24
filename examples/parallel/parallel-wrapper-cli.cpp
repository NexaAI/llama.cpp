#include "parallel-wrapper.h"
#include "common.h"

#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <random>

static std::string k_system =
    R"(Transcript of a never ending dialog, where the User interacts with an Assistant.
The Assistant is helpful, kind, honest, good at writing, and never fails to answer the User's requests immediately and with precision.

User: Recommend a nice restaurant in the area.
Assistant: I recommend the restaurant "The Golden Duck". It is a 5 star restaurant with a great view of the city. The food is delicious and the service is excellent. The prices are reasonable and the portions are generous. The restaurant is located at 123 Main Street, New York, NY 10001. The phone number is (212) 555-1234. The hours are Monday through Friday from 11:00 am to 10:00 pm. The restaurant is closed on Saturdays and Sundays.
User: Who is Richard Feynman?
Assistant: Richard Feynman was an American physicist who is best known for his work in quantum mechanics and particle physics. He was awarded the Nobel Prize in Physics in 1965 for his contributions to the development of quantum electrodynamics. He was a popular lecturer and author, and he wrote several books, including "Surely You're Joking, Mr. Feynman!" and "What Do You Care What Other People Think?".
User:)";

static std::vector<std::string> k_prompts = {
    "What is the meaning of life?",
    "Tell me an interesting fact about llamas.",
    "What is the best way to cook a steak?",
    "Are you familiar with the Special Theory of Relativity and can you explain it to me?",
    "Recommend some interesting books to read.",
    "What is the best way to learn a new language?",
    "How to get a job at Google?",
    "If you could have any superpower, what would it be?",
    "I want to learn how to play the piano.",
};

static std::vector<std::string> get_random_prompts(const std::vector<std::string> &all_prompts, size_t n)
{
    std::vector<std::string> shuffled_prompts = all_prompts;
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(shuffled_prompts.begin(), shuffled_prompts.end(), g);
    return std::vector<std::string>(shuffled_prompts.begin(), shuffled_prompts.begin() + n);
}

static char **get_random_prompts_c_strings(const std::vector<std::string> &all_prompts, size_t n)
{
    std::vector<std::string> prompts = get_random_prompts(all_prompts, n);

    char **c_strings = new char *[n];
    for (size_t i = 0; i < n; ++i)
    {
        c_strings[i] = strdup(prompts[i].c_str());
    }
    return c_strings;
}

int main(int argc, char **argv)
{
    parallel_context_params params = parallel_context_default_params();
    if (!parallel_context_params_parse(argc, argv, params))
    {
        return 1;
    }

    params.system_prompt = k_system.c_str();

    parallel_context *parallel_ctx = parallel_init_context(params);

    // Get a random subset of prompts
    char **prompts = get_random_prompts_c_strings(k_prompts, params.n_parallel);
    char **responses = parallel_inference(parallel_ctx, prompts);

    for (size_t i = 0; i < params.n_parallel; ++i)
    {
        LOG_TEE("Prompt %zu: %s\n", i, prompts[i]);
        LOG_TEE("Response %zu: %s\n", i, responses[i]);
    }

    // Get a different random subset of prompts
    prompts = get_random_prompts_c_strings(k_prompts, params.n_parallel);
    responses = parallel_inference(parallel_ctx, prompts);

    for (size_t i = 0; i < params.n_parallel; ++i)
    {
        LOG_TEE("Prompt %zu: %s\n", i, prompts[i]);
        LOG_TEE("Response %zu: %s\n", i, responses[i]);
    }

    parallel_free(parallel_ctx);

    return 0;
}