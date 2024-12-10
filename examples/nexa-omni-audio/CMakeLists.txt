# Find the Threads package
find_package(Threads REQUIRED)

# Build nexa-whisper-utils
set(WHISPER_LIB nexa-whisper-utils)
add_library(${WHISPER_LIB} OBJECT
            whisper.cpp
            )
target_link_libraries(${WHISPER_LIB} PRIVATE ggml llama common Threads::Threads)

# Add omni_audio library
set(OMNI_AUDIO_LIB omni_audio)
add_library(${OMNI_AUDIO_LIB} STATIC
            omni.cpp
            omni.h
            audio-projector.cpp
            audio-projector.h
            )
target_link_libraries(${OMNI_AUDIO_LIB} PRIVATE ggml llama common ${WHISPER_LIB})

<<<<<<< Updated upstream
# Include directories for omni_audio
=======
>>>>>>> Stashed changes
target_include_directories(${OMNI_AUDIO_LIB} PUBLIC .)
target_include_directories(${OMNI_AUDIO_LIB} PUBLIC ../..)
target_include_directories(${OMNI_AUDIO_LIB} PUBLIC ../../common)

<<<<<<< Updated upstream
# Compile features for omni_audio
target_compile_features(${OMNI_AUDIO_LIB} PRIVATE cxx_std_11)

# Create a static library from omni_audio
=======
target_compile_features(${OMNI_AUDIO_LIB} PRIVATE cxx_std_11)

>>>>>>> Stashed changes
add_library(omni_audio_static STATIC $<TARGET_OBJECTS:${OMNI_AUDIO_LIB}>)

# Shared library creation
if (BUILD_SHARED_LIBS)
    message(STATUS "Building shared libraries")

    # Ensure position-independent code for shared libraries
    set_target_properties(${OMNI_AUDIO_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(omni_audio_static PROPERTIES POSITION_INDEPENDENT_CODE ON)

    # Create the shared library
    add_library(omni_audio_shared SHARED $<TARGET_OBJECTS:${OMNI_AUDIO_LIB}>)
    target_link_libraries(omni_audio_shared PRIVATE common ggml llama ${WHISPER_LIB} Threads::Threads)

    # Set target properties for shared library
    set_target_properties(omni_audio_shared PROPERTIES
        PUBLIC_HEADER omni.h
        POSITION_INDEPENDENT_CODE ON
        OUTPUT_NAME "omni_audio_shared"
    )

    # Install the shared library
    install(TARGETS omni_audio_shared 
        LIBRARY
        PUBLIC_HEADER DESTINATION include
    )

    # Add compile definitions for the shared library
    target_compile_definitions(omni_audio_shared PRIVATE OMNI_AUDIO_SHARED WHISPER_SHARED)

    # Ensure all symbols are exported on Windows
    if(MSVC)
        set_target_properties(omni_audio_shared PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()
<<<<<<< Updated upstream
endif()

# Build the nexa-omni-cli executable
add_executable(nexa-omni-cli omni-cli.cpp)
target_link_libraries(nexa-omni-cli PRIVATE ggml llama common Threads::Threads ${WHISPER_LIB} omni_audio_static)
=======
endif()
>>>>>>> Stashed changes
