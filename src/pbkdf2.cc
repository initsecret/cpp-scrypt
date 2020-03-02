// pbkdf2.cc - A wrapper around OpenSSL's PBKDF2 function.

#include "pbkdf2.h"

#include <cstdlib>
#include <iostream>
#include <limits>

#include "utilities.h"

pbkdf2::pbkdf2(const EVP_MD* d) : digest{d} {}

std::vector<std::byte> pbkdf2::hash(std::vector<std::byte> passphrase,
                                    std::vector<std::byte> salt,
                                    uint32_t iterations,
                                    size_t desired_length) {
  // Turn input into format expected by OpenSSL
  auto passphrase_as_chars = new char[passphrase.size()];
  for (size_t i = 0; i < passphrase.size(); ++i) {
    passphrase_as_chars[i] = static_cast<char>(passphrase.at(i));
  }
  auto salt_as_uchars = new unsigned char[salt.size()];
  for (size_t i = 0; i < salt.size(); ++i) {
    salt_as_uchars[i] = static_cast<unsigned char>(salt.at(i));
  }
  auto output = new unsigned char[desired_length];

  if (!output) {
    std::cout << "Could not allocate output buffer.\n";
    assert(false);
  }

  if (iterations >= std::numeric_limits<int>::max()) {
    std::cout << "More iterations than INT_MAX.\n";
    assert(false);
  }

  if (desired_length >= std::numeric_limits<int>::max()) {
    std::cout << "desired_length larger than INT_MAX.\n";
    assert(false);
  }

  if (passphrase.size() >= std::numeric_limits<int>::max()) {
    std::cout << "passphrase.size() larger than INT_MAX.\n";
    assert(false);
  }

  if (salt.size() >= std::numeric_limits<int>::max()) {
    std::cout << "salt.size() larger than INT_MAX.\n";
    assert(false);
  }

  int res = PKCS5_PBKDF2_HMAC(
      passphrase_as_chars, static_cast<int>(passphrase.size()), salt_as_uchars,
      static_cast<int>(salt.size()), static_cast<int>(iterations), digest,
      static_cast<int>(desired_length), output);

  if (res == 0) {
    std::cout << "PKCS5_PBKDF2_HMAC failed.\n";
    assert(false);
  }

  std::vector<std::byte> output_vector = {};

  for (size_t i = 0; i < desired_length; ++i) {
    output_vector.push_back(static_cast<std::byte>(output[i]));
  }

  delete[] output;
  delete[] salt_as_uchars;
  delete[] passphrase_as_chars;

  return output_vector;
}
