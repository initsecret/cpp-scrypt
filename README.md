# A C++ Implementation of Scrypt

![C++ CMake CI](https://github.com/sgmenda/cpp-scrypt/workflows/C++%20CMake%20CI/badge.svg)

A C++ implementation of scrypt ([RFC 7914](https://datatracker.ietf.org/doc/rfc7914/)). A C++ implementation of the [Salsa20 hash function](https://cr.yp.to/snuffle/spec.pdf) is included, but the PBDKF2 is from OpenSSL.

**Requires:** [OpenSSL](https://www.openssl.org/) and [GMP](https://gmplib.org/).

**License:** BSD-3.

**Disclaimer:** NOT FOR PRODUCTION! (This code most certainly contains bugs; is not efficient; and no effort had been made to make it resistant to side-channel attacks.)
