# below CMakeLists.txt still has some issues, Qwen2Audio is not working properly

# Find the Threads package
find_package(Threads REQUIRED)
if(NOT Threads_FOUND)
    message(FATAL_ERROR "Failed to find Threads package")
endif()

# build whisper-utils
set(WHISPER_LIB nexa-whisper-utils)
add_library(${WHISPER_LIB} OBJECT whisper.cpp grammar-parser.cpp)
target_link_libraries(${WHISPER_LIB} PRIVATE ggml common Threads::Threads)

# build the OMNI_AUDIO_LIB
set(OMNI_AUDIO_LIB nexa-omni-audio-lib)
add_library(${OMNI_AUDIO_LIB} OBJECT
            omni.cpp
            omni.h
            audio-projector.cpp
            audio-projector.h
            )
target_link_libraries(${OMNI_AUDIO_LIB} PRIVATE ggml common Threads::Threads ${WHISPER_LIB})

# build the omni-cli
add_executable(nexa-omni-cli omni-cli.cpp)
target_link_libraries(nexa-omni-cli PRIVATE ggml common Threads::Threads ${WHISPER_LIB} ${OMNI_AUDIO_LIB})

# build the QWEN2_AUDIO_LIB
set(QWEN2_AUDIO_LIB nexa-qwen2-audio-lib)
add_library(${QWEN2_AUDIO_LIB} OBJECT
            qwen2audio.cpp
            qwen2audio.h
            audio-projector.cpp
            audio-projector.h
            )
target_link_libraries(${QWEN2_AUDIO_LIB} PRIVATE ggml common Threads::Threads ${WHISPER_LIB})

# build the qwen2-cli
add_executable(nexa-qwen2-audio-cli qwen2audio-cli.cpp)
target_link_libraries(nexa-qwen2-audio-cli PRIVATE ggml common Threads::Threads ${WHISPER_LIB} ${QWEN2_AUDIO_LIB})

# If BUILD_SHARED_LIBS is ON, also build a shared library
if(BUILD_SHARED_LIBS)
    message(STATUS "Building audio LLM shared libraries")

    # Set position independent code for all libraries
    set_target_properties(${WHISPER_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(${OMNI_AUDIO_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(${QWEN2_AUDIO_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)

    # Create shared library for OMNI_AUDIO_LIB
    add_library(${OMNI_AUDIO_LIB}_shared SHARED $<TARGET_OBJECTS:${OMNI_AUDIO_LIB}>)
    target_link_libraries(${OMNI_AUDIO_LIB}_shared PRIVATE ggml common ${WHISPER_LIB})
    set_target_properties(${OMNI_AUDIO_LIB}_shared PROPERTIES
        PUBLIC_HEADER omni.h
        POSITION_INDEPENDENT_CODE ON
    )

    # Create shared library for QWEN2_AUDIO_LIB
    add_library(${QWEN2_AUDIO_LIB}_shared SHARED $<TARGET_OBJECTS:${QWEN2_AUDIO_LIB}>)
    target_link_libraries(${QWEN2_AUDIO_LIB}_shared PRIVATE ggml common ${WHISPER_LIB})
    set_target_properties(${QWEN2_AUDIO_LIB}_shared PROPERTIES
        PUBLIC_HEADER qwen2audio.h
        POSITION_INDEPENDENT_CODE ON
    )

    # Add shared library definitions
    target_compile_definitions(${OMNI_AUDIO_LIB}_shared PRIVATE OMNI_AUDIO_SHARED WHISPER_SHARED)
    target_compile_definitions(${QWEN2_AUDIO_LIB}_shared PRIVATE QWEN2_AUDIO_SHARED WHISPER_SHARED)

    # Ensure all symbols are exported on Windows
    if(MSVC)
        set_target_properties(${OMNI_AUDIO_LIB}_shared PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
        set_target_properties(${QWEN2_AUDIO_LIB}_shared PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()
endif()