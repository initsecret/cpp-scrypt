cmake_minimum_required(VERSION 3.5)

project(cpp-scrypt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# use openssl
set(OPENSSL_ROOT_DIR /usr/local/opt/openssl/)
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

## use GMP
find_library(GMP gmp)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

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
