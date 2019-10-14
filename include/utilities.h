#ifndef UTILITIES_H
#define UTILITIES_H

#include <cstddef>
#include <vector>

class utilities
{
public:
  utilities();

  static std::string bytesToHex(std::vector<std::byte> data);
  static std::vector<std::byte> hexToBytes(std::string hex_string);

  static std::vector<std::byte> stringToBytes(std::string s);
};

#endif // UTILITIES_H
