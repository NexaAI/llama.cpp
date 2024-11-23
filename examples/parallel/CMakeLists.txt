set(TARGET llama-parallel)
add_executable(${TARGET} parallel.cpp)
install(TARGETS ${TARGET} RUNTIME)
target_link_libraries(${TARGET} PRIVATE common llama ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(${TARGET} PRIVATE cxx_std_11)

set(PARALLEL_LIB parallel-wrapper)
add_library(${PARALLEL_LIB} OBJECT
    parallel-wrapper.cpp
    parallel-wrapper.h
)
target_link_libraries(${PARALLEL_LIB} PRIVATE common llama ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(${PARALLEL_LIB} PRIVATE cxx_std_11)

add_executable(${TARGET}-cli parallel-wrapper-cli.cpp)
install(TARGETS ${TARGET}-cli RUNTIME)
target_link_libraries(${TARGET}-cli PRIVATE common llama ${CMAKE_THREAD_LIBS_INIT} ${PARALLEL_LIB})
target_compile_features(${TARGET}-cli PRIVATE cxx_std_11)

if(BUILD_SHARED_LIBS)
    message(STATUS "Building shared libraries")

    set_target_properties(${PARALLEL_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    add_library(${PARALLEL_LIB}_shared SHARED $<TARGET_OBJECTS:${PARALLEL_LIB}>)
    target_link_libraries(${PARALLEL_LIB}_shared PRIVATE common llama)

    set_target_properties(${PARALLEL_LIB}_shared PROPERTIES
        PUBLIC_HEADER parallel-wrapper.h
        POSITION_INDEPENDENT_CODE ON
        OUTPUT_NAME "${PARALLEL_LIB}_shared"
    )
    install(TARGETS ${PARALLEL_LIB}_shared
        LIBRARY
        PUBLIC_HEADER DESTINATION include
    )

    target_compile_definitions(${PARALLEL_LIB}_shared PRIVATE PARALLEL_SHARED)

    if(MSVC)
        set_target_properties(${PARALLEL_LIB}_shared PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()
endif()
