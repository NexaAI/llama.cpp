# whisper

# Find the Threads package
find_package(Threads REQUIRED)

# build whisper-utils
set(WHISPER_LIB nexa-whisper-utils-qwen2)
add_library(${WHISPER_LIB} OBJECT whisper.cpp grammar-parser.cpp)
target_link_libraries(${WHISPER_LIB} PRIVATE ggml common Threads::Threads)

# build the QWEN2_AUDIO_LIB
set(QWEN2_AUDIO_LIB nexa-qwen2-audio-lib)
add_library(${QWEN2_AUDIO_LIB} OBJECT
            qwen2audio.cpp
            qwen2audio.h
            audio-projector.cpp
            audio-projector.h
            )
target_link_libraries(${QWEN2_AUDIO_LIB} PRIVATE ggml  common Threads::Threads ${WHISPER_LIB})

# build the qwen2-cli
add_executable(nexa-qwen2-audio-cli qwen2audio-cli.cpp)
target_link_libraries(nexa-qwen2-audio-cli PRIVATE ggml  common Threads::Threads ${WHISPER_LIB} ${QWEN2_AUDIO_LIB})
install(TARGETS nexa-qwen2-audio-cli RUNTIME)
target_compile_features(nexa-qwen2-audio-cli PRIVATE cxx_std_11)

# If BUILD_SHARED_LIBS is ON, also build a shared library
message(STATUS "Building audio LLM shared libraries")

# Set position independent code for all libraries
set_target_properties(${WHISPER_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(${QWEN2_AUDIO_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Create shared library for QWEN2_AUDIO_LIB
add_library(${QWEN2_AUDIO_LIB}_shared SHARED $<TARGET_OBJECTS:${QWEN2_AUDIO_LIB}>)
target_link_libraries(${QWEN2_AUDIO_LIB}_shared PRIVATE ggml  common ${WHISPER_LIB})
set_target_properties(${QWEN2_AUDIO_LIB}_shared PROPERTIES
    PUBLIC_HEADER qwen2audio.h
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME "${QWEN2_AUDIO_LIB}"
)
install(TARGETS ${QWEN2_AUDIO_LIB}_shared 
    LIBRARY
    PUBLIC_HEADER DESTINATION include
)

# Add shared library definitions
target_compile_definitions(${QWEN2_AUDIO_LIB}_shared PRIVATE QWEN2_AUDIO_SHARED WHISPER_SHARED)

# Ensure all symbols are exported on Windows
if(MSVC)
    set_target_properties(${QWEN2_AUDIO_LIB}_shared PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()