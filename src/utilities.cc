// utilities.cc - Some utilities.

#include "utilities.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

utilities::utilities() {}

// Given a vector of bytes, returns a hex string of the bytes
string utilities::bytesToHex(vector<byte> data) {
  stringstream ss;
  ss << std::hex;

  int col = 0;
  for (byte data_item : data) {
    int data_int = static_cast<int>(data_item);
    ss << setw(1) << ((data_int & 0xf0) >> 4);
    ss << setw(1) << (data_int & 0x0f);
    ss << " ";
    col++;
    if ((col % 16) == 0) {
      ss << endl;
    }
  }
  ss << endl;

  return ss.str();
}

// Given a hex string, return a vector of bytes.
// Adapted from https://stackoverflow.com/a/3221193
std::vector<std::byte> utilities::hexToBytes(std::string hex_string) {
  std::istringstream hex_string_stream(hex_string);
  std::vector<std::byte> data;

  unsigned int c;
  while (hex_string_stream >> std::hex >> c) {
    data.push_back(static_cast<std::byte>(c));
  }

  return data;
}

// Given a string, return the corresponding vector of bytes.
std::vector<std::byte> utilities::stringToBytes(std::string s) {
  std::vector<std::byte> data;

  for (size_t i = 0; i < s.size(); ++i) {
    data.push_back(static_cast<std::byte>(s[i]));
  }

  return data;
}
