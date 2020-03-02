// salsa_test.cc - Some tests for Salsa

#include <gtest/gtest.h>
#include <salsa20.h>

#include "utilities.h"

namespace {
//
// Examples from https://cr.yp.to/snuffle/spec.pdf
//

// Test primitives like quarterround
TEST(SalsaTest, TestPrimitives) {
  Salsa20 Salsa(20);
  EXPECT_EQ(Salsa.test_primitives(), 0);
}

// Hash of all zeros
TEST(SalsaTest, AllZeroHash) {
  Salsa20 Salsa(20);
  std::vector<std::byte> all_zeros{
      std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
      std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
      std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
      std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
      std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
      std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
      std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
      std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
      std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
      std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
      std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
      std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
      std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}};
  std::vector<std::byte> got = Salsa.hash(all_zeros);
  EXPECT_EQ(all_zeros, got);
}

// Hash of random data 0
TEST(SalsaTest, RandomDataHash0) {
  Salsa20 Salsa(20);
  std::vector<std::byte> input{
      std::byte{211}, std::byte{159}, std::byte{13},  std::byte{115},
      std::byte{76},  std::byte{55},  std::byte{82},  std::byte{183},
      std::byte{3},   std::byte{117}, std::byte{222}, std::byte{37},
      std::byte{191}, std::byte{187}, std::byte{234}, std::byte{136},
      std::byte{49},  std::byte{237}, std::byte{179}, std::byte{48},
      std::byte{1},   std::byte{106}, std::byte{178}, std::byte{219},
      std::byte{175}, std::byte{199}, std::byte{166}, std::byte{48},
      std::byte{86},  std::byte{16},  std::byte{179}, std::byte{207},
      std::byte{31},  std::byte{240}, std::byte{32},  std::byte{63},
      std::byte{15},  std::byte{83},  std::byte{93},  std::byte{161},
      std::byte{116}, std::byte{147}, std::byte{48},  std::byte{113},
      std::byte{238}, std::byte{55},  std::byte{204}, std::byte{36},
      std::byte{79},  std::byte{201}, std::byte{235}, std::byte{79},
      std::byte{3},   std::byte{81},  std::byte{156}, std::byte{47},
      std::byte{203}, std::byte{26},  std::byte{244}, std::byte{243},
      std::byte{88},  std::byte{118}, std::byte{104}, std::byte{54}};
  std::vector<std::byte> expected{
      std::byte{109}, std::byte{42},  std::byte{178}, std::byte{168},
      std::byte{156}, std::byte{240}, std::byte{248}, std::byte{238},
      std::byte{168}, std::byte{196}, std::byte{190}, std::byte{203},
      std::byte{26},  std::byte{110}, std::byte{170}, std::byte{154},
      std::byte{29},  std::byte{29},  std::byte{150}, std::byte{26},
      std::byte{150}, std::byte{30},  std::byte{235}, std::byte{249},
      std::byte{190}, std::byte{163}, std::byte{251}, std::byte{48},
      std::byte{69},  std::byte{144}, std::byte{51},  std::byte{57},
      std::byte{118}, std::byte{40},  std::byte{152}, std::byte{157},
      std::byte{180}, std::byte{57},  std::byte{27},  std::byte{94},
      std::byte{107}, std::byte{42},  std::byte{236}, std::byte{35},
      std::byte{27},  std::byte{111}, std::byte{114}, std::byte{114},
      std::byte{219}, std::byte{236}, std::byte{232}, std::byte{135},
      std::byte{111}, std::byte{155}, std::byte{110}, std::byte{18},
      std::byte{24},  std::byte{232}, std::byte{95},  std::byte{158},
      std::byte{179}, std::byte{19},  std::byte{48},  std::byte{202}};
  std::vector<std::byte> got = Salsa.hash(input);
  EXPECT_EQ(expected, got);
}

// Hash of random data 1
TEST(SalsaTest, RandomDataHash1) {
  Salsa20 Salsa(20);
  std::vector<std::byte> input{
      std::byte{88},  std::byte{118}, std::byte{104}, std::byte{54},
      std::byte{79},  std::byte{201}, std::byte{235}, std::byte{79},
      std::byte{3},   std::byte{81},  std::byte{156}, std::byte{47},
      std::byte{203}, std::byte{26},  std::byte{244}, std::byte{243},
      std::byte{191}, std::byte{187}, std::byte{234}, std::byte{136},
      std::byte{211}, std::byte{159}, std::byte{13},  std::byte{115},
      std::byte{76},  std::byte{55},  std::byte{82},  std::byte{183},
      std::byte{3},   std::byte{117}, std::byte{222}, std::byte{37},
      std::byte{86},  std::byte{16},  std::byte{179}, std::byte{207},
      std::byte{49},  std::byte{237}, std::byte{179}, std::byte{48},
      std::byte{1},   std::byte{106}, std::byte{178}, std::byte{219},
      std::byte{175}, std::byte{199}, std::byte{166}, std::byte{48},
      std::byte{238}, std::byte{55},  std::byte{204}, std::byte{36},
      std::byte{31},  std::byte{240}, std::byte{32},  std::byte{63},
      std::byte{15},  std::byte{83},  std::byte{93},  std::byte{161},
      std::byte{116}, std::byte{147}, std::byte{48},  std::byte{113}};
  std::vector<std::byte> expected{
      std::byte{179}, std::byte{19},  std::byte{48},  std::byte{202},
      std::byte{219}, std::byte{236}, std::byte{232}, std::byte{135},
      std::byte{111}, std::byte{155}, std::byte{110}, std::byte{18},
      std::byte{24},  std::byte{232}, std::byte{95},  std::byte{158},
      std::byte{26},  std::byte{110}, std::byte{170}, std::byte{154},
      std::byte{109}, std::byte{42},  std::byte{178}, std::byte{168},
      std::byte{156}, std::byte{240}, std::byte{248}, std::byte{238},
      std::byte{168}, std::byte{196}, std::byte{190}, std::byte{203},
      std::byte{69},  std::byte{144}, std::byte{51},  std::byte{57},
      std::byte{29},  std::byte{29},  std::byte{150}, std::byte{26},
      std::byte{150}, std::byte{30},  std::byte{235}, std::byte{249},
      std::byte{190}, std::byte{163}, std::byte{251}, std::byte{48},
      std::byte{27},  std::byte{111}, std::byte{114}, std::byte{114},
      std::byte{118}, std::byte{40},  std::byte{152}, std::byte{157},
      std::byte{180}, std::byte{57},  std::byte{27},  std::byte{94},
      std::byte{107}, std::byte{42},  std::byte{236}, std::byte{35}};
  std::vector<std::byte> got = Salsa.hash(input);
  EXPECT_EQ(expected, got);
}

// A Crap tonne of rounds
TEST(SalsaTest, TadTooManyRounds) {
  Salsa20 Salsa(20);
  std::vector<std::byte> input{
      std::byte{6},   std::byte{124}, std::byte{83},  std::byte{146},
      std::byte{38},  std::byte{191}, std::byte{9},   std::byte{50},
      std::byte{4},   std::byte{161}, std::byte{47},  std::byte{222},
      std::byte{122}, std::byte{182}, std::byte{223}, std::byte{185},
      std::byte{75},  std::byte{27},  std::byte{0},   std::byte{216},
      std::byte{16},  std::byte{122}, std::byte{7},   std::byte{89},
      std::byte{162}, std::byte{104}, std::byte{101}, std::byte{147},
      std::byte{213}, std::byte{21},  std::byte{54},  std::byte{95},
      std::byte{225}, std::byte{253}, std::byte{139}, std::byte{176},
      std::byte{105}, std::byte{132}, std::byte{23},  std::byte{116},
      std::byte{76},  std::byte{41},  std::byte{176}, std::byte{207},
      std::byte{221}, std::byte{34},  std::byte{157}, std::byte{108},
      std::byte{94},  std::byte{94},  std::byte{99},  std::byte{52},
      std::byte{90},  std::byte{117}, std::byte{91},  std::byte{220},
      std::byte{146}, std::byte{190}, std::byte{239}, std::byte{143},
      std::byte{196}, std::byte{176}, std::byte{130}, std::byte{186}};
  std::vector<std::byte> expected{
      std::byte{8},   std::byte{18},  std::byte{38},  std::byte{199},
      std::byte{119}, std::byte{76},  std::byte{215}, std::byte{67},
      std::byte{173}, std::byte{127}, std::byte{144}, std::byte{162},
      std::byte{103}, std::byte{212}, std::byte{176}, std::byte{217},
      std::byte{192}, std::byte{19},  std::byte{233}, std::byte{33},
      std::byte{159}, std::byte{197}, std::byte{154}, std::byte{160},
      std::byte{128}, std::byte{243}, std::byte{219}, std::byte{65},
      std::byte{171}, std::byte{136}, std::byte{135}, std::byte{225},
      std::byte{123}, std::byte{11},  std::byte{68},  std::byte{86},
      std::byte{237}, std::byte{82},  std::byte{20},  std::byte{155},
      std::byte{133}, std::byte{189}, std::byte{9},   std::byte{83},
      std::byte{167}, std::byte{116}, std::byte{194}, std::byte{78},
      std::byte{122}, std::byte{127}, std::byte{195}, std::byte{185},
      std::byte{185}, std::byte{204}, std::byte{188}, std::byte{90},
      std::byte{245}, std::byte{9},   std::byte{183}, std::byte{248},
      std::byte{226}, std::byte{85},  std::byte{245}, std::byte{104}};

  std::vector<std::byte> got = Salsa.hash(input);
  for (int i = 1; i < 1000000; i++) {
    got = Salsa.hash(got);
  }
  EXPECT_EQ(expected, got);
}

//
// From Section 8 of the Scrypt RFC
//
TEST(SalsaTest, ScryptRFCSanity0) {
  Salsa20 Salsa(8);
  std::string input =
      "7e 87 9a 21 4f 3e c9 86 7c a9 40 e6 41 71 8f 26 "
      "ba ee 55 5b 8c 61 c1 b5 0d f8 46 11 6d cd 3b 1d "
      "ee 24 f3 19 df 9b 3d 85 14 12 1e 4b 5a c5 aa 32 "
      "76 02 1d 29 09 c7 48 29 ed eb c6 8d b8 b8 c2 5e ";
  std::string expected =
      "a4 1f 85 9c 66 08 cc 99 3b 81 ca cb 02 0c ef 05 "
      "04 4b 21 81 a2 fd 33 7d fd 7b 1c 63 96 68 2f 29 "
      "b4 39 31 68 e3 c9 e6 bc fe 6b c5 b7 a0 6d 96 ba "
      "e4 24 cc 10 2c 91 74 5c 24 ad 67 3d c7 61 8f 81 ";

  std::vector<std::byte> got = Salsa.hash(utilities::hexToBytes(input));

  EXPECT_EQ(got, utilities::hexToBytes(expected));
}

}  // namespace
