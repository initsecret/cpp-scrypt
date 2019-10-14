// scrypt.cc - An implementation of scrypt.
// Based on Wikipedia's pseudocode
// (https://en.wikipedia.org/wiki/Scrypt#Algorithm) and the scrypt spec (RFC
// 7914)

#include "scrypt.h"

#include "pbkdf2.h"
#include "salsa20.h"
#include "utilities.h"

#include <gmpxx.h>
#include <iostream>
#include <string>

scrypt::scrypt() = default;

// Adapted from https://stackoverflow.com/a/24018122/8327793
mpz_class
Integrify(std::vector<std::byte> uint_as_bytes)
{
  auto p = std::make_shared<mpz_class>();
  mpz_import(
    p->get_mpz_t(), uint_as_bytes.size(), -1, 1, -1, 0, &uint_as_bytes[0]);

  return *p;
}

uint64_t
IntegrifyModN(std::vector<std::vector<std::byte>> B,
              unsigned long cost_factor_N)
{
  size_t two_r = B.size();
  std::vector<std::byte> last_B = B.at(two_r - 1);

  assert(last_B.size() >= 8);

  mpz_class integrified = Integrify(last_B);

  integrified = integrified % mpz_class(cost_factor_N);

  return integrified.get_ui();
}

std::vector<std::byte>
BlockXOR(std::vector<std::byte> A, std::vector<std::byte> B)
{
  std::vector<std::byte> C;
  assert(A.size() == B.size());
  for (size_t i = 0; i < A.size(); ++i) {
    C.push_back(A.at(i) ^ B.at(i));
  }
  return C;
}

std::vector<std::vector<std::byte>>
BlockVectorXOR(std::vector<std::vector<std::byte>> A,
               std::vector<std::vector<std::byte>> B)
{
  std::vector<std::vector<std::byte>> C;
  assert(A.size() == B.size());
  for (size_t i = 0; i < A.size(); ++i) {
    C.push_back(BlockXOR(A.at(i), B.at(i)));
  }
  return C;
}

std::vector<std::vector<std::byte>>
BlockMix(std::vector<std::vector<std::byte>> B)
{
  size_t two_r = B.size();
  std::vector<std::byte> X = B.at(two_r - 1);
  std::vector<std::vector<std::byte>> Y(two_r, { static_cast<std::byte>(0) });

  salsa20 salsa20_8(8);

  for (size_t i = 0; i < two_r; i++) {
    auto T = BlockXOR(X, B.at(i));
    X = salsa20_8.hash(T);
    Y.at(i) = X;
  }

  std::vector<std::vector<std::byte>> output_vector;

  for (size_t i = 0; i < two_r; i += 2) {
    // Even blocks first
    output_vector.push_back(Y.at(i));
  }
  for (size_t i = 1; i < two_r; i += 2) {
    // Then odd blocks
    output_vector.push_back(Y.at(i));
  }

  return output_vector;
}

std::vector<std::byte>
ROMix(uint32_t block_size_factor_r,
      std::vector<std::byte> block,
      uint64_t cost_factor_N)
{
  // interpret block as 2r 64-byte chunks.
  size_t block_size = 64;
  size_t two_r = block.size() / block_size;

  assert((2 * block_size_factor_r) == two_r);

  std::vector<std::vector<std::byte>> B;
  for (size_t i = 0; i < two_r; i++) {
    auto it = block.begin();
    std::vector<std::byte> Bi(it + (i * 64), it + ((i + 1) * 64));
    B.push_back(Bi);
  }

  std::vector<std::vector<std::byte>> X(B);
  std::vector<std::vector<std::vector<std::byte>>> V;

  for (uint64_t i = 0; i < cost_factor_N; ++i) {
    V.push_back(X);
    X = BlockMix(X);
  }

  for (uint64_t i = 0; i < cost_factor_N; ++i) {
    uint64_t j = IntegrifyModN(X, cost_factor_N);
    auto T = BlockVectorXOR(X, V.at(j));
    X = BlockMix(T);
  }

  std::vector<std::byte> B_out = {};

  for (size_t i = 0; i < two_r; i++) {
    auto it = B_out.begin() + (i * block_size);
    B_out.insert(it, X.at(i).begin(), X.at(i).end());
    assert(X.at(i).size() == block_size);
  }

  return B_out;
}

std::vector<std::byte>
scrypt::hash(std::vector<std::byte> passphrase,
             std::vector<std::byte> salt,
             uint64_t cost_factor_N,
             uint32_t block_size_factor_r,
             uint32_t parallelization_factor_p,
             size_t desired_key_length)
{
  //
  // 1. Generate an expensive salt using PBKDF2
  //

  uint32_t block_size = 128 * block_size_factor_r;

  pbkdf2 PBKDF2_SHA256(EVP_sha256());

  std::vector<std::byte> expensive_salt = PBKDF2_SHA256.hash(
    passphrase, salt, 1, block_size * parallelization_factor_p);

  // Let us split expensive salt into B0, B1,...,B(p-1) each with block_size
  // bytes.
  std::vector<std::vector<std::byte>> B;
  for (size_t i = 0; i < parallelization_factor_p; i++) {
    auto it = expensive_salt.begin();
    std::vector<std::byte> Bi(it + (i * block_size),
                              it + ((i + 1) * block_size));
    B.push_back(Bi);
  }

  // Let us mix these blocks
  std::vector<std::vector<std::byte>> mixed_B;
  for (size_t i = 0; i < parallelization_factor_p; i++) {
    mixed_B.push_back(ROMix(block_size_factor_r, B.at(i), cost_factor_N));
  }

  std::vector<std::byte> mixed_expensive_salt = {};

  for (size_t i = 0; i < parallelization_factor_p; i++) {
    auto it = mixed_expensive_salt.begin() + (i * block_size);
    mixed_expensive_salt.insert(it, mixed_B.at(i).begin(), mixed_B.at(i).end());
    assert(mixed_B.at(i).size() == block_size);
  }

  //
  // 2. Use PBKDF2 and the expensive salt to generate the hash
  //

  std::vector<std::byte> output_buffer =
    PBKDF2_SHA256.hash(passphrase, mixed_expensive_salt, 1, desired_key_length);

  return output_buffer;
}

void
scrypt::test(bool run_long_tests)
{
  // From Section 8 of the RFC
  salsa20 salsa20_8(8);
  std::string salsa_in_1 = "7e 87 9a 21 4f 3e c9 86 7c a9 40 e6 41 71 8f 26 "
                           "ba ee 55 5b 8c 61 c1 b5 0d f8 46 11 6d cd 3b 1d "
                           "ee 24 f3 19 df 9b 3d 85 14 12 1e 4b 5a c5 aa 32 "
                           "76 02 1d 29 09 c7 48 29 ed eb c6 8d b8 b8 c2 5e ";
  std::vector<std::byte> salsa_in_1_v = utilities::hexToBytes(salsa_in_1);
  std::vector<std::byte> salsa_out_1_v = salsa20_8.hash(salsa_in_1_v);
  std::string salsa_out_1 = "a4 1f 85 9c 66 08 cc 99 3b 81 ca cb 02 0c ef 05 "
                            "04 4b 21 81 a2 fd 33 7d fd 7b 1c 63 96 68 2f 29 "
                            "b4 39 31 68 e3 c9 e6 bc fe 6b c5 b7 a0 6d 96 ba "
                            "e4 24 cc 10 2c 91 74 5c 24 ad 67 3d c7 61 8f 81 ";
  assert(salsa_out_1_v == utilities::hexToBytes(salsa_out_1));

  // From Section 9 of the RFC
  std::string blockmix_in_0_0 =
    "f7 ce 0b 65 3d 2d 72 a4 10 8c f5 ab e9 12 ff dd "
    "77 76 16 db bb 27 a7 0e 82 04 f3 ae 2d 0f 6f ad "
    "89 f6 8f 48 11 d1 e8 7b cc 3b d7 40 0a 9f fd 29 "
    "09 4f 01 84 63 95 74 f3 9a e5 a1 31 52 17 bc d7 ";
  std::string blockmix_in_0_1 =
    "89 49 91 44 72 13 bb 22 6c 25 b5 4d a8 63 70 fb "
    "cd 98 43 80 37 46 66 bb 8f fc b5 bf 40 c2 54 b0 "
    "67 d2 7c 51 ce 4a d5 fe d8 29 c9 0b 50 5a 57 1b "
    "7f 4d 1c ad 6a 52 3c da 77 0e 67 bc ea af 7e 89 ";

  std::vector<std::vector<std::byte>> blockmix_in_0{
    utilities::hexToBytes(blockmix_in_0_0),
    utilities::hexToBytes(blockmix_in_0_1)
  };
  std::vector<std::vector<std::byte>> blockmix_out_0 = BlockMix(blockmix_in_0);
  std::string blockmix_out_0_0 =
    "a4 1f 85 9c 66 08 cc 99 3b 81 ca cb 02 0c ef 05 "
    "04 4b 21 81 a2 fd 33 7d fd 7b 1c 63 96 68 2f 29 "
    "b4 39 31 68 e3 c9 e6 bc fe 6b c5 b7 a0 6d 96 ba "
    "e4 24 cc 10 2c 91 74 5c 24 ad 67 3d c7 61 8f 81 ";
  std::string blockmix_out_0_1 =
    "20 ed c9 75 32 38 81 a8 05 40 f6 4c 16 2d cd 3c "
    "21 07 7c fe 5f 8d 5f e2 b1 a4 16 8f 95 36 78 b7 "
    "7d 3b 3d 80 3b 60 e4 ab 92 09 96 e5 9b 4d 53 b6 "
    "5d 2a 22 58 77 d5 ed f5 84 2c b9 f1 4e ef e4 25 ";
  std::vector<std::vector<std::byte>> expected_blockmix_out_0{
    utilities::hexToBytes(blockmix_out_0_0),
    utilities::hexToBytes(blockmix_out_0_1)
  };
  assert(blockmix_out_0 == expected_blockmix_out_0);

  // From Section 10 of the RFC
  std::string romix_in_0 = "f7 ce 0b 65 3d 2d 72 a4 10 8c f5 ab e9 12 ff dd "
                           "77 76 16 db bb 27 a7 0e 82 04 f3 ae 2d 0f 6f ad "
                           "89 f6 8f 48 11 d1 e8 7b cc 3b d7 40 0a 9f fd 29 "
                           "09 4f 01 84 63 95 74 f3 9a e5 a1 31 52 17 bc d7 "
                           "89 49 91 44 72 13 bb 22 6c 25 b5 4d a8 63 70 fb "
                           "cd 98 43 80 37 46 66 bb 8f fc b5 bf 40 c2 54 b0 "
                           "67 d2 7c 51 ce 4a d5 fe d8 29 c9 0b 50 5a 57 1b "
                           "7f 4d 1c ad 6a 52 3c da 77 0e 67 bc ea af 7e 89 ";
  std::vector<std::byte> romix_out_0 =
    ROMix(1, utilities::hexToBytes(romix_in_0), 16);
  std::string expected_romix_out_0 =
    "79 cc c1 93 62 9d eb ca 04 7f 0b 70 60 4b f6 b6 "
    "2c e3 dd 4a 96 26 e3 55 fa fc 61 98 e6 ea 2b 46 "
    "d5 84 13 67 3b 99 b0 29 d6 65 c3 57 60 1f b4 26 "
    "a0 b2 f4 bb a2 00 ee 9f 0a 43 d1 9b 57 1a 9c 71 "
    "ef 11 42 e6 5d 5a 26 6f dd ca 83 2c e5 9f aa 7c "
    "ac 0b 9c f1 be 2b ff ca 30 0d 01 ee 38 76 19 c4 "
    "ae 12 fd 44 38 f2 03 a0 e4 e1 c4 7e c3 14 86 1f "
    "4e 90 87 cb 33 39 6a 68 73 e8 f9 d2 53 9a 4b 8e ";
  assert(romix_out_0 == utilities::hexToBytes(expected_romix_out_0));

  // From Section 11 of the RFC
  pbkdf2 testPBKDF(EVP_sha256());
  std::vector<std::byte> pbkdf_out_0 =
    testPBKDF.hash(utilities::stringToBytes("passwd"),
                   utilities::stringToBytes("salt"),
                   1,
                   64);
  std::string expected_pbkdf_out_0 =
    "55 ac 04 6e 56 e3 08 9f ec 16 91 c2 25 44 b6 05 "
    "f9 41 85 21 6d de 04 65 e6 8b 9d 57 c2 0d ac bc "
    "49 ca 9c cc f1 79 b6 45 99 16 64 b3 9d 77 ef 31 "
    "7c 71 b8 45 b1 e3 0b d5 09 11 20 41 d3 a1 97 83 ";
  assert(pbkdf_out_0 == utilities::hexToBytes(expected_pbkdf_out_0));
  std::vector<std::byte> pbkdf_out_1 =
    testPBKDF.hash(utilities::stringToBytes("Password"),
                   utilities::stringToBytes("NaCl"),
                   80000,
                   64);
  std::string expected_pbkdf_out_1 =
    "4d dc d8 f6 0b 98 be 21 83 0c ee 5e f2 27 01 f9 "
    "64 1a 44 18 d0 4c 04 14 ae ff 08 87 6b 34 ab 56 "
    "a1 d4 25 a1 22 58 33 54 9a db 84 1b 51 c9 b3 17 "
    "6a 27 2b de bb a1 d0 78 47 8f 62 b3 97 f3 3c 8d ";
  assert(pbkdf_out_1 == utilities::hexToBytes(expected_pbkdf_out_1));

  // From Section 12 of the RFC
  scrypt testScrypt;

  std::vector<std::byte> scrypt_out_0 = testScrypt.hash(
    utilities::stringToBytes(""), utilities::stringToBytes(""), 16, 1, 1, 64);
  std::string expected_scrypt_out_0 =
    "77 d6 57 62 38 65 7b 20 3b 19 ca 42 c1 8a 04 97 "
    "f1 6b 48 44 e3 07 4a e8 df df fa 3f ed e2 14 42 "
    "fc d0 06 9d ed 09 48 f8 32 6a 75 3a 0f c8 1f 17 "
    "e8 d3 e0 fb 2e 0d 36 28 cf 35 e2 0c 38 d1 89 06 ";
  assert(scrypt_out_0 == utilities::hexToBytes(expected_scrypt_out_0));

  std::vector<std::byte> scrypt_out_1 =
    testScrypt.hash(utilities::stringToBytes("password"),
                    utilities::stringToBytes("NaCl"),
                    1024,
                    8,
                    16,
                    64);
  std::string expected_scrypt_out_1 =
    "fd ba be 1c 9d 34 72 00 78 56 e7 19 0d 01 e9 fe "
    "7c 6a d7 cb c8 23 78 30 e7 73 76 63 4b 37 31 62 "
    "2e af 30 d9 2e 22 a3 88 6f f1 09 27 9d 98 30 da "
    "c7 27 af b9 4a 83 ee 6d 83 60 cb df a2 cc 06 40 ";
  assert(scrypt_out_1 == utilities::hexToBytes(expected_scrypt_out_1));

  if (run_long_tests == true) {
    std::vector<std::byte> scrypt_out_2 =
      testScrypt.hash(utilities::stringToBytes("pleaseletmein"),
                      utilities::stringToBytes("SodiumChloride"),
                      16384,
                      8,
                      1,
                      64);
    std::string expected_scrypt_out_2 =
      "70 23 bd cb 3a fd 73 48 46 1c 06 cd 81 fd 38 eb "
      "fd a8 fb ba 90 4f 8e 3e a9 b5 43 f6 54 5d a1 f2 "
      "d5 43 29 55 61 3f 0f cf 62 d4 97 05 24 2a 9a f9 "
      "e6 1e 85 dc 0d 65 1e 40 df cf 01 7b 45 57 58 87 ";

    std::cout << utilities::bytesToHex(scrypt_out_2);
    assert(scrypt_out_2 == utilities::hexToBytes(expected_scrypt_out_2));

    std::vector<std::byte> scrypt_out_3 =
      testScrypt.hash(utilities::stringToBytes("pleaseletmein"),
                      utilities::stringToBytes("SodiumChloride"),
                      1048576,
                      8,
                      1,
                      64);
    std::string expected_scrypt_out_3 =
      "21 01 cb 9b 6a 51 1a ae ad db be 09 cf 70 f8 81 "
      "ec 56 8d 57 4a 2f fd 4d ab e5 ee 98 20 ad aa 47 "
      "8e 56 fd 8f 4b a5 d0 9f fa 1c 6d 92 7c 40 f4 c3 "
      "37 30 40 49 e8 a9 52 fb cb f4 5c 6f a7 7a 41 a4 ";
    assert(scrypt_out_3 == utilities::hexToBytes(expected_scrypt_out_3));
  }
}
