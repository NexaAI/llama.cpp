# whisper

# Find the Threads package
find_package(Threads REQUIRED)

# build whisper-utils
set(WHISPER_LIB nexa-whisper-utils)
add_library(${WHISPER_LIB} OBJECT whisper.cpp grammar-parser.cpp)
target_link_libraries(${WHISPER_LIB} PRIVATE ggml  common Threads::Threads)

# build the OMNI_AUDIO_LIB
set(OMNI_AUDIO_LIB nexa-omni-audio-lib)
add_library(${OMNI_AUDIO_LIB} OBJECT
            omni.cpp
            omni.h
            audio-projector.cpp
            audio-projector.h
            )
target_link_libraries(${OMNI_AUDIO_LIB} PRIVATE ggml  common Threads::Threads ${WHISPER_LIB})

# build the omni-cli
add_executable(nexa-omni-cli omni-cli.cpp)
target_link_libraries(nexa-omni-cli PRIVATE ggml  common Threads::Threads ${WHISPER_LIB} ${OMNI_AUDIO_LIB})
install(TARGETS nexa-omni-cli RUNTIME)
target_compile_features(nexa-omni-cli PRIVATE cxx_std_11)

# Set position independent code for all libraries
set_target_properties(${WHISPER_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(${OMNI_AUDIO_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Create shared library for OMNI_AUDIO_LIB
add_library(${OMNI_AUDIO_LIB}_shared SHARED $<TARGET_OBJECTS:${OMNI_AUDIO_LIB}>)
target_link_libraries(${OMNI_AUDIO_LIB}_shared PRIVATE ggml  common ${WHISPER_LIB})
set_target_properties(${OMNI_AUDIO_LIB}_shared PROPERTIES
    PUBLIC_HEADER omni.h
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME "${OMNI_AUDIO_LIB}"
)
install(TARGETS ${OMNI_AUDIO_LIB}_shared 
    LIBRARY
    PUBLIC_HEADER DESTINATION include
)

# Add shared library definitions
target_compile_definitions(${OMNI_AUDIO_LIB}_shared PRIVATE OMNI_AUDIO_SHARED WHISPER_SHARED)

# Ensure all symbols are exported on Windows
if(MSVC)
    set_target_properties(${OMNI_AUDIO_LIB}_shared PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()