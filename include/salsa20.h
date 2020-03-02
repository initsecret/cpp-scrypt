#ifndef SALSA20_H
#define SALSA20_H

#include <cstddef>
#include <cstdint>
#include <vector>

class Salsa20 {
 private:
  uint8_t rounds;

 public:
  Salsa20(uint8_t rounds = 20);

  std::vector<std::byte> hash(std::vector<std::byte> message);

  int test_primitives();
};

#endif  // SALSA20_H
