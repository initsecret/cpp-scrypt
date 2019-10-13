#ifndef SCRYPT_H
#define SCRYPT_H

#include <cstddef>
#include <cstdint>
#include <vector>

class scrypt
{
public:
  // Eventually, I want to modify this to take in a PRF and a MF as in MFcrypt
  // algorithm in [SCRYPT]. For now, we take a the scrypt defaults (HMAC_SHA256,
  // ROMMix).
  scrypt();

  void test(bool run_long_tests = false);

  std::vector<std::byte> hash(std::vector<std::byte> passphrase,
                              std::vector<std::byte> salt,
                              uint64_t cost_factor_N,
                              uint32_t block_size_factor_r,
                              uint32_t parallelization_factor_p,
                              size_t desired_key_length);
};

#endif // SCRYPT_H
