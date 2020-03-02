#ifndef SCRYPT_H
#define SCRYPT_H

#include <cstddef>
#include <cstdint>
#include <vector>

class Scrypt {
 public:
  // Eventually, I want to modify this to take in a PRF and a MF as in MFcrypt
  // algorithm in [SCRYPT]. For now, we use the scrypt defaults (HMAC_SHA256,
  // ROMMix).
  Scrypt();

  std::vector<std::byte> hash(std::vector<std::byte> passphrase,
                              std::vector<std::byte> salt,
                              uint64_t cost_factor_N,
                              uint32_t block_size_factor_r,
                              uint32_t parallelization_factor_p,
                              size_t desired_key_length);

  int test_primitives();
};

#endif  // SCRYPT_H
