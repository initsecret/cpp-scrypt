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

set(
    SOURCES
    main.cc
    scrypt.h
    scrypt.cc
    salsa20.h
    salsa20.cc
    pbkdf2.h
    pbkdf2.cc
    utilities.h
    utilities.cc
    )

add_executable(main ${SOURCES})
target_link_libraries(main OpenSSL::Crypto)
#target_link_libraries(main gmpxx)
#target_link_libraries(main gmp)
# Linking gmp the old school way.
target_link_libraries(main "${GMP}")
