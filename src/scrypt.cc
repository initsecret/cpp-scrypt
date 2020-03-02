// scrypt.cc - An implementation of scrypt.
// Based on Wikipedia's pseudocode
// (https://en.wikipedia.org/wiki/Scrypt#Algorithm) and the scrypt spec (RFC
// 7914)

#include "scrypt.h"

#include <gmpxx.h>

#include <cassert>
#include <iostream>
#include <memory>
#include <string>

#include "pbkdf2.h"
#include "salsa20.h"
#include "utilities.h"

Scrypt::Scrypt() = default;

// Adapted from https://stackoverflow.com/a/24018122/8327793
mpz_class Integrify(std::vector<std::byte> uint_as_bytes) {
  auto p = std::make_shared<mpz_class>();
  mpz_import(p->get_mpz_t(), uint_as_bytes.size(), -1, 1, -1, 0,
             &uint_as_bytes[0]);

  return *p;
}

uint64_t IntegrifyModN(std::vector<std::vector<std::byte>> B,
                       unsigned long cost_factor_N) {
  size_t two_r = B.size();
  std::vector<std::byte> last_B = B.at(two_r - 1);

  assert(last_B.size() >= 8);

  mpz_class integrified = Integrify(last_B);

  integrified = integrified % mpz_class(cost_factor_N);

  return integrified.get_ui();
}

std::vector<std::byte> BlockXOR(std::vector<std::byte> A,
                                std::vector<std::byte> B) {
  std::vector<std::byte> C;
  assert(A.size() == B.size());
  for (size_t i = 0; i < A.size(); ++i) {
    C.push_back(A.at(i) ^ B.at(i));
  }
  return C;
}

std::vector<std::vector<std::byte>> BlockVectorXOR(
    std::vector<std::vector<std::byte>> A,
    std::vector<std::vector<std::byte>> B) {
  std::vector<std::vector<std::byte>> C;
  assert(A.size() == B.size());
  for (size_t i = 0; i < A.size(); ++i) {
    C.push_back(BlockXOR(A.at(i), B.at(i)));
  }
  return C;
}

std::vector<std::vector<std::byte>> BlockMix(
    std::vector<std::vector<std::byte>> B) {
  size_t two_r = B.size();
  std::vector<std::byte> X = B.at(two_r - 1);
  std::vector<std::vector<std::byte>> Y(two_r, {static_cast<std::byte>(0)});

  Salsa20 salsa20_8(8);

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

std::vector<std::byte> ROMix(uint32_t block_size_factor_r,
                             std::vector<std::byte> block,
                             uint64_t cost_factor_N) {
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

std::vector<std::byte> Scrypt::hash(std::vector<std::byte> passphrase,
                                    std::vector<std::byte> salt,
                                    uint64_t cost_factor_N,
                                    uint32_t block_size_factor_r,
                                    uint32_t parallelization_factor_p,
                                    size_t desired_key_length) {
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

  std::vector<std::byte> output_buffer = PBKDF2_SHA256.hash(
      passphrase, mixed_expensive_salt, 1, desired_key_length);

  return output_buffer;
}

int Scrypt::test_primitives() {
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
      utilities::hexToBytes(blockmix_in_0_1)};
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
      utilities::hexToBytes(blockmix_out_0_1)};
  assert(blockmix_out_0 == expected_blockmix_out_0);

  // From Section 10 of the RFC
  std::string romix_in_0 =
      "f7 ce 0b 65 3d 2d 72 a4 10 8c f5 ab e9 12 ff dd "
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
  return 0;
}
