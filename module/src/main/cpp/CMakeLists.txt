cmake_minimum_required(VERSION 3.22.1)
project("soulbypass")

set(CMAKE_CXX_STANDARD 17)

# Find Android log library
find_library(log-lib log)

# Check for Dobby
set(DOBBY_LIB "${CMAKE_CURRENT_SOURCE_DIR}/libs/libdobby.a")
if(EXISTS ${DOBBY_LIB})
    message(STATUS "Dobby found: ${DOBBY_LIB}")
    add_library(dobby STATIC IMPORTED)
    set_target_properties(dobby PROPERTIES
        IMPORTED_LOCATION ${DOBBY_LIB}
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    set(HAS_DOBBY TRUE)
else()
    message(WARNING "Dobby not found, building without hook support")
    set(HAS_DOBBY FALSE)
endif()

# Main library
add_library(soulbypass SHARED
    src/main.cpp
    src/pc_cache.cpp
    src/seccomp_filter.cpp
    src/memfd_faker.cpp
    src/dlopen_hook.cpp
)

target_include_directories(soulbypass PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/zygisk
)

if(HAS_DOBBY)
    target_link_libraries(soulbypass
        ${log-lib}
        dobby
    )
    target_compile_definitions(soulbypass PRIVATE HAS_DOBBY=1)
else()
    target_link_libraries(soulbypass
        ${log-lib}
    )
    target_compile_definitions(soulbypass PRIVATE HAS_DOBBY=0)
endif()

# Compiler flags
target_compile_options(soulbypass PRIVATE
    -O2
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -fno-exceptions
    -fno-rtti
    -Wall
    -Wextra
)

# Strip symbols in release
set_target_properties(soulbypass PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
