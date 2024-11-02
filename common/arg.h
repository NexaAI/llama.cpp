#pragma once  // Ensures this header is only included once in the compilation unit, avoiding multiple definition errors.

#include "common.h"   // Include common header file, likely contains shared definitions and includes.

#include <set>        // Include the C++ standard library set container.
#include <string>     // Include the C++ standard string class.
#include <vector>     // Include the C++ standard vector container.

//
// CLI argument parsing
//

// Struct for handling command-line arguments.
struct llama_arg {
    std::set<enum llama_example> examples = {LLAMA_EXAMPLE_COMMON}; // Stores different example cases for arguments.
    std::vector<const char *> args; // List of argument names or aliases.
    const char * value_hint   = nullptr; // Optional help text or example for the argument's value.
    const char * value_hint_2 = nullptr; // Optional help text for a second argument value (if applicable).
    const char * env          = nullptr; // Stores an environment variable name that may provide the value for this argument.
    std::string help;  // Help message that describes the argument.
    bool is_sparam = false;  // Boolean flag to mark if this argument is a sampling parameter.
    
    // Function pointers to handle different argument types during parsing.
    void (*handler_void)   (gpt_params & params) = nullptr;  // Handles void type (no additional argument required).
    void (*handler_string) (gpt_params & params, const std::string &) = nullptr;  // Handles string arguments.
    void (*handler_str_str)(gpt_params & params, const std::string &, const std::string &) = nullptr;  // Handles two string arguments.
    void (*handler_int)    (gpt_params & params, int) = nullptr;  // Handles integer arguments.

    // Constructor for arguments that take a single string value.
    llama_arg(
        const std::initializer_list<const char *> & args,  // List of possible names for this argument (e.g., "-f", "--file").
        const char * value_hint,  // Optional example or help text for the argument value.
        const std::string & help,  // Help message for this argument.
        void (*handler)(gpt_params & params, const std::string &)  // Function to handle this argument, which takes a string value.
    ) : args(args), value_hint(value_hint), help(help), handler_string(handler) {}

    // Constructor for arguments that take an integer value.
    llama_arg(
        const std::initializer_list<const char *> & args,  // List of argument names.
        const char * value_hint,  // Example or hint for the expected integer value.
        const std::string & help,  // Help message for this argument.
        void (*handler)(gpt_params & params, int)  // Function to handle this argument, which takes an integer value.
    ) : args(args), value_hint(value_hint), help(help), handler_int(handler) {}

    // Constructor for arguments that don't require additional values (void).
    llama_arg(
        const std::initializer_list<const char *> & args,  // List of argument names.
        const std::string & help,  // Help message for this argument.
        void (*handler)(gpt_params & params)  // Function to handle this argument without a value (void).
    ) : args(args), help(help), handler_void(handler) {}

    // Constructor for arguments that take two string values.
    llama_arg(
        const std::initializer_list<const char *> & args,  // List of argument names.
        const char * value_hint,  // Hint for the first value.
        const char * value_hint_2,  // Hint for the second value.
        const std::string & help,  // Help message for this argument.
        void (*handler)(gpt_params & params, const std::string &, const std::string &)  // Function to handle this argument with two string values.
    ) : args(args), value_hint(value_hint), value_hint_2(value_hint_2), help(help), handler_str_str(handler) {}

    // Method to set examples for this argument, returning a reference to the current llama_arg object (for method chaining).
    llama_arg & set_examples(std::initializer_list<enum llama_example> examples);

    // Method to associate this argument with an environment variable.
    llama_arg & set_env(const char * env);

    // Method to mark this argument as a sampling parameter.
    llama_arg & set_sparam();

    // Checks if the argument is included in the given example.
    bool in_example(enum llama_example ex);

    // Retrieves the value of the argument from the associated environment variable.
    bool get_value_from_env(std::string & output);

    // Checks if the environment variable provides a value for this argument.
    bool has_value_from_env();

    // Converts the argument and its details to a string for display (e.g., when printing help or error messages).
    std::string to_string();
};

// Struct for holding the context of GPT parameters and argument parsing.
struct gpt_params_context {
    enum llama_example ex = LLAMA_EXAMPLE_COMMON;  // Example flag, indicates which example set to use for argument parsing.
    gpt_params & params;  // Reference to the parameters struct that stores the values set by the arguments.
    std::vector<llama_arg> options;  // List of all possible arguments (options) for this context.
    void(*print_usage)(int, char **) = nullptr;  // Function pointer for printing usage instructions.

    // Constructor that initializes the context with a reference to the gpt_params.
    gpt_params_context(gpt_params & params) : params(params) {}
};

// Parses command-line arguments (argc, argv) into the gpt_params struct.
// If an argument is invalid, it will automatically print usage instructions for that specific argument.
bool gpt_params_parse(int argc, char ** argv, gpt_params & params, llama_example ex, void(*print_usage)(int, char **) = nullptr);

// Initializes a gpt_params_context for testing or parsing with a specific example and print_usage function.
gpt_params_context gpt_params_parser_init(gpt_params & params, llama_example ex, void(*print_usage)(int, char **) = nullptr);
