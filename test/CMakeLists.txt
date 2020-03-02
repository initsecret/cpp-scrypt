# Test Salsa
add_executable(salsa_test salsa_test.cc)
target_link_libraries(salsa_test gtest_main)
target_link_libraries(salsa_test cpp-scrypt)
add_test(NAME salsa_test COMMAND salsa_test)

# Test PBKDF2
add_executable(pbkdf_test pbkdf_test.cc)
target_link_libraries(pbkdf_test gtest_main)
target_link_libraries(pbkdf_test cpp-scrypt)
add_test(NAME pbkdf_test COMMAND pbkdf_test)

# Test Scrypt
add_executable(scrypt_test scrypt_test.cc)
target_link_libraries(scrypt_test gtest_main)
target_link_libraries(scrypt_test cpp-scrypt)
add_test(NAME scrypt_test COMMAND scrypt_test)
