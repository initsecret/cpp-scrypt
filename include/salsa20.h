#ifndef SALSA20_H
#define SALSA20_H

#include <cstdint>
#include <vector>

class salsa20
{
private:
  uint8_t rounds;

public:
  salsa20(uint8_t rounds);

  void test(bool run_long_tests = false);

  std::vector<std::byte> hash(std::vector<std::byte> message);
};

#endif // SALSA20_H
