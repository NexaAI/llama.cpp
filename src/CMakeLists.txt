# TODO: should not use this
if (WIN32)
    if (BUILD_SHARED_LIBS)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()
endif()

# Add a library target called "llama". This can be a shared or static library, depending on the build configuration.
# The library is composed of the following source files and headers:
# - llama.h (from the include directory)
# - llama.cpp (implementation)
# - llama-vocab.cpp (handles vocabulary-related tasks)
# - llama-grammar.cpp (parses grammar structures)
# - llama-sampling.cpp (deals with sampling logic)
# - unicode.h and unicode.cpp (handle Unicode processing)
# - unicode-data.cpp (Unicode data support)
add_library(llama
            ../include/llama.h   # Header for Llama's public API
            llama.cpp            # Core Llama implementation
            llama-vocab.cpp      # Vocabulary-related code
            llama-grammar.cpp    # Grammar handling code
            llama-sampling.cpp   # Sampling strategies
            unicode.h            # Header for Unicode processing
            unicode.cpp          # Implementation for Unicode processing
            unicode-data.cpp     # Data for Unicode support
)

# Specifies directories that should be included during compilation.
# This makes the current directory (".") and "../include" available for header file lookups.
target_include_directories(llama PUBLIC . ../include)

# Ensures that the `llama` target is compiled using the C++11 standard. 
# This prevents future upgrades beyond C++11, ensuring compatibility.
target_compile_features(llama PUBLIC cxx_std_11) # don't bump

# Links the `ggml` library to the `llama` target.
# This means the `llama` library depends on and will use symbols from the `ggml` library.
target_link_libraries(llama PUBLIC ggml)

# If shared libraries are being built (instead of static), some additional properties are required.
if (BUILD_SHARED_LIBS)
    # Position-independent code is necessary for shared libraries, as they need to be relocatable.
    set_target_properties(llama PROPERTIES POSITION_INDEPENDENT_CODE ON)
    
    # Defines preprocessor macros `LLAMA_SHARED` and `LLAMA_BUILD` when compiling the llama target.
    # These are typically used to control how functions and variables are exported/imported in shared libraries.
    target_compile_definitions(llama PRIVATE LLAMA_SHARED LLAMA_BUILD)
endif()
