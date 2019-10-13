#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "salsa20.h"
#include "scrypt.h"

using namespace std;

int
main()
{
  salsa20 TestSalsa(8);
  TestSalsa.test();

  scrypt TestScrypt;
  TestScrypt.test();
}
