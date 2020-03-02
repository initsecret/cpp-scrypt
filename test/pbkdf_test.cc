// pbkdf_test.cc - Some tests for PBKDF2

#include <gtest/gtest.h>
#include <pbkdf2.h>

#include "utilities.h"

namespace {
// From Section 11 of the Scrypt RFC
TEST(PBKDF2Test, ScryptRFCSanity0) {
  pbkdf2 PBKDF(EVP_sha256());
  std::vector<std::byte> pbkdf_out_0 =
      PBKDF.hash(utilities::stringToBytes("passwd"),
                 utilities::stringToBytes("salt"), 1, 64);
  std::string expected_pbkdf_out_0 =
      "55 ac 04 6e 56 e3 08 9f ec 16 91 c2 25 44 b6 05 "
      "f9 41 85 21 6d de 04 65 e6 8b 9d 57 c2 0d ac bc "
      "49 ca 9c cc f1 79 b6 45 99 16 64 b3 9d 77 ef 31 "
      "7c 71 b8 45 b1 e3 0b d5 09 11 20 41 d3 a1 97 83 ";
  EXPECT_EQ(pbkdf_out_0, utilities::hexToBytes(expected_pbkdf_out_0));
}

TEST(PBKDF2Test, ScryptRFCSanity1) {
  pbkdf2 PBKDF(EVP_sha256());
  std::vector<std::byte> pbkdf_out_1 =
      PBKDF.hash(utilities::stringToBytes("Password"),
                 utilities::stringToBytes("NaCl"), 80000, 64);
  std::string expected_pbkdf_out_1 =
      "4d dc d8 f6 0b 98 be 21 83 0c ee 5e f2 27 01 f9 "
      "64 1a 44 18 d0 4c 04 14 ae ff 08 87 6b 34 ab 56 "
      "a1 d4 25 a1 22 58 33 54 9a db 84 1b 51 c9 b3 17 "
      "6a 27 2b de bb a1 d0 78 47 8f 62 b3 97 f3 3c 8d ";
  EXPECT_EQ(pbkdf_out_1, utilities::hexToBytes(expected_pbkdf_out_1));
}

}  // namespace
