// salsa20.cc - An implementation of Salsa hash.
// Based on the spec (https://cr.yp.to/snuffle/spec.pdf).

#include "salsa20.h"

#include <cassert>
#include <iomanip>
#include <iostream>

// This function is defined in Section 2 of the spec.
// out_i = y_{i + (c mod 32)} (where y_i is the ith bit of y.)
uint32_t leftRotation(uint32_t y, uint8_t c) {
  return ((y << c) | (y >> (32 - c)));
}

std::vector<uint32_t> quarterround(std::vector<uint32_t> y) {
  assert(y.size() == 4);

  std::vector<uint32_t> z(4, 0);

  z.at(1) = y.at(1) ^ leftRotation(y.at(0) + y.at(3), 7);
  z.at(2) = y.at(2) ^ leftRotation(z.at(1) + y.at(0), 9);
  z.at(3) = y.at(3) ^ leftRotation(z.at(2) + z.at(1), 13);
  z.at(0) = y.at(0) ^ leftRotation(z.at(3) + z.at(2), 18);

  return z;
}

std::vector<uint32_t> rowround(std::vector<uint32_t> y) {
  assert(y.size() == 16);

  std::vector<uint32_t> z(16, 0);

  std::vector<uint32_t> q1 = quarterround({y.at(0), y.at(1), y.at(2), y.at(3)});
  z.at(0) = q1.at(0);
  z.at(1) = q1.at(1);
  z.at(2) = q1.at(2);
  z.at(3) = q1.at(3);

  std::vector<uint32_t> q2 = quarterround({y.at(5), y.at(6), y.at(7), y.at(4)});
  z.at(5) = q2.at(0);
  z.at(6) = q2.at(1);
  z.at(7) = q2.at(2);
  z.at(4) = q2.at(3);

  std::vector<uint32_t> q3 =
      quarterround({y.at(10), y.at(11), y.at(8), y.at(9)});
  z.at(10) = q3.at(0);
  z.at(11) = q3.at(1);
  z.at(8) = q3.at(2);
  z.at(9) = q3.at(3);

  std::vector<uint32_t> q4 =
      quarterround({y.at(15), y.at(12), y.at(13), y.at(14)});
  z.at(15) = q4.at(0);
  z.at(12) = q4.at(1);
  z.at(13) = q4.at(2);
  z.at(14) = q4.at(3);

  return z;
}

std::vector<uint32_t> columnround(std::vector<uint32_t> x) {
  assert(x.size() == 16);

  std::vector<uint32_t> y(16, 0);

  std::vector<uint32_t> q1 =
      quarterround({x.at(0), x.at(4), x.at(8), x.at(12)});
  y.at(0) = q1.at(0);
  y.at(4) = q1.at(1);
  y.at(8) = q1.at(2);
  y.at(12) = q1.at(3);

  std::vector<uint32_t> q2 =
      quarterround({x.at(5), x.at(9), x.at(13), x.at(1)});
  y.at(5) = q2.at(0);
  y.at(9) = q2.at(1);
  y.at(13) = q2.at(2);
  y.at(1) = q2.at(3);

  std::vector<uint32_t> q3 =
      quarterround({x.at(10), x.at(14), x.at(2), x.at(6)});
  y.at(10) = q3.at(0);
  y.at(14) = q3.at(1);
  y.at(2) = q3.at(2);
  y.at(6) = q3.at(3);

  std::vector<uint32_t> q4 =
      quarterround({x.at(15), x.at(3), x.at(7), x.at(11)});
  y.at(15) = q4.at(0);
  y.at(3) = q4.at(1);
  y.at(7) = q4.at(2);
  y.at(11) = q4.at(3);

  return y;
}

std::vector<uint32_t> doubleround(std::vector<uint32_t> x) {
  assert(x.size() == 16);
  return rowround(columnround(x));
}

// Turns 4 bytes into a little endian 32-bit uint
uint32_t littleendian(std::vector<uint8_t> b) {
  assert(b.size() == 4);

  uint32_t x = 0;

  for (size_t i = 0; i < 4; ++i) {
    x = x | (b.at(i) << (8 * i));
  }

  return x;
}

// Turns a little endian 32-bit uint into 4 bytes
std::vector<uint8_t> littleendianInverse(uint32_t x) {
  std::vector<uint8_t> b = {};
  b.push_back((x & 0x000000ff));
  b.push_back((x & 0x0000ff00) >> 8);
  b.push_back((x & 0x00ff0000) >> 16);
  b.push_back((x & 0xff000000) >> 24);

  return b;
}

std::vector<uint8_t> salsa_hash(std::vector<uint8_t> input_x, uint32_t rounds) {
  assert(input_x.size() == 64);
  assert((rounds % 2) == 0);
  uint32_t num_doublerounds = rounds / 2;

  std::vector<uint32_t> x;

  auto it = input_x.begin();

  for (int i = 0; i < 64; i += 4) {
    std::vector<uint8_t> input_x_subvector(it, it + 4);
    it += 4;

    x.push_back(littleendian(input_x_subvector));
  }

  std::vector<uint32_t> z(x);  // deep copy x into z.

  for (uint32_t i = 0; i < num_doublerounds; ++i) {
    z = doubleround(z);
  }

  std::vector<uint8_t> z_output;

  for (size_t i = 0; i < 16; ++i) {
    std::vector<uint8_t> inv = littleendianInverse(x.at(i) + z.at(i));
    for (auto inv_item : inv) {
      z_output.push_back(inv_item);
    }
  }

  return z_output;
}

int internal_test_primitives() {
  // An example from Section 2
  uint32_t y = 0xc0a8787e;
  uint32_t o = leftRotation(y, 5);
  assert(o == 0x150f0fd8);

  // Examples from Section 3
  std::vector<uint32_t> e1{0, 0, 0, 0};
  assert(quarterround({0, 0, 0, 0}) == e1);

  std::vector<uint32_t> e2{0x8008145, 0x80, 0x10200, 0x20500000};
  assert(quarterround({1, 0, 0, 0}) == e2);

  std::vector<uint32_t> e3{0x88000100, 0x00000001, 0x00000200, 0x00402000};
  assert(quarterround({0, 1, 0, 0}) == e3);

  std::vector<uint32_t> e4{0x80040000, 0x00000000, 0x00000001, 0x00002000};
  assert(quarterround({0, 0, 1, 0}) == e4);

  std::vector<uint32_t> e5{0x00048044, 0x00000080, 0x00010000, 0x20100001};
  assert(quarterround({0, 0, 0, 1}) == e5);

  std::vector<uint32_t> e6{0xe876d72b, 0x9361dfd5, 0xf1460244, 0x948541a3};
  assert(quarterround({0xe7e8c006, 0xc4f9417d, 0x6479b4b2, 0x68c67137}) == e6);

  std::vector<uint32_t> e7{0x3e2f308c, 0xd90a8f36, 0x6ab2a923, 0x2883524c};
  assert(quarterround({0xd3917c5b, 0x55f1c407, 0x52a58a7a, 0x8f887a3b}) == e7);

  // Examples from Section 4
  std::vector<uint32_t> rr1{0x08008145, 0x00000080, 0x00010200, 0x20500000,
                            0x20100001, 0x00048044, 0x00000080, 0x00010000,
                            0x00000001, 0x00002000, 0x80040000, 0x00000000,
                            0x00000001, 0x00000200, 0x00402000, 0x88000100};
  assert(rowround({0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000001,
                   0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000000,
                   0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x00000000,
                   0x00000000}) == rr1);

  std::vector<uint32_t> rr2{0xa890d39d, 0x65d71596, 0xe9487daa, 0xc8ca6a86,
                            0x949d2192, 0x764b7754, 0xe408d9b9, 0x7a41b4d1,
                            0x3402e183, 0x3c3af432, 0x50669f96, 0xd89ef0a8,
                            0x0040ede5, 0xb545fbce, 0xd257ed4f, 0x1818882d};
  assert(rowround({0x08521bd6, 0x1fe88837, 0xbb2aa576, 0x3aa26365, 0xc54c6a5b,
                   0x2fc74c2f, 0x6dd39cc3, 0xda0a64f6, 0x90a2f23d, 0x067f95a6,
                   0x06b35f61, 0x41e4732e, 0xe859c100, 0xea4d84b7, 0x0f619bff,
                   0xbc6e965a}) == rr2);

  // Examples from Section 5
  std::vector<uint32_t> cr1{0x10090288, 0x00000000, 0x00000000, 0x00000000,
                            0x00000101, 0x00000000, 0x00000000, 0x00000000,
                            0x00020401, 0x00000000, 0x00000000, 0x00000000,
                            0x40a04001, 0x00000000, 0x00000000, 0x00000000};
  assert(columnround({0x00000001, 0x00000000, 0x00000000, 0x00000000,
                      0x00000001, 0x00000000, 0x00000000, 0x00000000,
                      0x00000001, 0x00000000, 0x00000000, 0x00000000,
                      0x00000001, 0x00000000, 0x00000000, 0x00000000}) == cr1);

  std::vector<uint32_t> cr2{0x8c9d190a, 0xce8e4c90, 0x1ef8e9d3, 0x1326a71a,
                            0x90a20123, 0xead3c4f3, 0x63a091a0, 0xf0708d69,
                            0x789b010c, 0xd195a681, 0xeb7d5504, 0xa774135c,
                            0x481c2027, 0x53a8e4b5, 0x4c1f89c5, 0x3f78c9c8};
  assert(columnround({0x08521bd6, 0x1fe88837, 0xbb2aa576, 0x3aa26365,
                      0xc54c6a5b, 0x2fc74c2f, 0x6dd39cc3, 0xda0a64f6,
                      0x90a2f23d, 0x067f95a6, 0x06b35f61, 0x41e4732e,
                      0xe859c100, 0xea4d84b7, 0x0f619bff, 0xbc6e965a}) == cr2);

  std::vector<uint32_t> dr1{0x8186a22d, 0x0040a284, 0x82479210, 0x06929051,
                            0x08000090, 0x02402200, 0x00004000, 0x00800000,
                            0x00010200, 0x20400000, 0x08008104, 0x00000000,
                            0x20500000, 0xa0000040, 0x0008180a, 0x612a8020};
  assert(doubleround({0x00000001, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000}) == dr1);

  std::vector<uint32_t> dr2{0xccaaf672, 0x23d960f7, 0x9153e63a, 0xcd9a60d0,
                            0x50440492, 0xf07cad19, 0xae344aa0, 0xdf4cfdfc,
                            0xca531c29, 0x8e7943db, 0xac1680cd, 0xd503ca00,
                            0xa74b2ad6, 0xbc331c5c, 0x1dda24c7, 0xee928277};
  assert(doubleround({0xde501066, 0x6f9eb8f7, 0xe4fbbd9b, 0x454e3f57,
                      0xb75540d3, 0x43e93a4c, 0x3a6f2aa0, 0x726d6b36,
                      0x9243f484, 0x9145d1e8, 0x4fa9d247, 0xdc8dee11,
                      0x054bf545, 0x254dd653, 0xd9421b6d, 0x67b276c1}) == dr2);

  // Examples from Section 7
  assert(littleendian({0, 0, 0, 0}) == 0x00000000);
  assert(littleendian({86, 75, 30, 9}) == 0x091e4b56);
  assert(littleendian({255, 255, 255, 250}) == 0xfaffffff);
  // and their inverses
  std::vector<uint8_t> i1{0, 0, 0, 0};
  assert(littleendianInverse(0x00000000) == i1);
  std::vector<uint8_t> i2{86, 75, 30, 9};
  assert(littleendianInverse(0x091e4b56) == i2);
  std::vector<uint8_t> i3{255, 255, 255, 250};
  assert(littleendianInverse(0xfaffffff) == i3);

  return 0;
}

//
// Wrappers
//

Salsa20::Salsa20(uint8_t r) {
  assert((r % 2) == 0);
  rounds = r;
}

int Salsa20::test_primitives() { return internal_test_primitives(); }

std::vector<std::byte> Salsa20::hash(std::vector<std::byte> message) {
  std::vector<uint8_t> message_uint_vector;
  for (auto m : message) {
    uint8_t n = static_cast<unsigned char>(m);
    message_uint_vector.push_back(n);
  }

  std::vector<uint8_t> hash_uint_vector =
      salsa_hash(message_uint_vector, rounds);

  std::vector<std::byte> output_vector;
  for (auto h : hash_uint_vector) {
    std::byte b = static_cast<std::byte>(h);
    output_vector.push_back(b);
  }

  return output_vector;
}
