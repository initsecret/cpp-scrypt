#ifndef PBKDF2_H
#define PBKDF2_H

#include <openssl/evp.h>

#include <vector>

class pbkdf2 {
  const EVP_MD* digest;

 public:
  pbkdf2(const EVP_MD* d);

  std::vector<std::byte> hash(std::vector<std::byte> passphrase,
                              std::vector<std::byte> salt, uint32_t iterations,
                              size_t desired_length);
};

#endif  // PBKDF2_H
