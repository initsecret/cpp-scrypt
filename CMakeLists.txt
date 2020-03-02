cmake_minimum_required(VERSION 3.5)

project(cpp-scrypt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# use openssl
# ==============================================================================
set(OPENSSL_ROOT_DIR /usr/local/opt/openssl/)
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

# ==============================================================================
# use GMP
# ==============================================================================
find_library(GMP gmp)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

# ==============================================================================
# use gtest
# from https://github.com/google/googletest/blob/master/googletest/README.md
# ==============================================================================
# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

# ==============================================================================
# ==============================================================================

include_directories(include)

set(
    SOURCES
    include/scrypt.h
    src/scrypt.cc
    include/salsa20.h
    src/salsa20.cc
    include/pbkdf2.h
    src/pbkdf2.cc
    include/utilities.h
    src/utilities.cc
    )

add_library(cpp-scrypt SHARED ${SOURCES})
set(PROJECT_VERSION 0.1)
set_target_properties(cpp-scrypt PROPERTIES VERSION ${PROJECT_VERSION})
target_link_libraries(cpp-scrypt OpenSSL::Crypto)
target_link_libraries(cpp-scrypt "${GMP}")

enable_testing()
add_subdirectory(test)
