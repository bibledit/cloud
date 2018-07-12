For adding secure transport to Bibledit, a number of SSL/TLS libraries were considered.

OpenSSL is widely used but for embedded applications it may be a bit bloated.

GnuTLS was als considered but not further looked into.

WolfSSL
https://www.wolfssl.com
https://github.com/wolfSSL/wolfssl-examples

Botan
http://botan.randombit.net/
It can generate a single amalgamation file:
./configure.py --gen-amalgamation --single-amalgamation-file
There's a lot of dependency on the processor architecture, including endian-ness and assembler code.
And this means that every CPU needs to be configured separately.

PolarSSL, now mbed TLS
https://tls.mbed.org/
This looks good, it is low level, loosely coupled, easy to understand.

MatrixSSL
http://www.matrixssl.org/
The code looks okay, but the implementation in the client is complicated, judging from the sample client application.

NanoSSL
https://www.mocana.com/iot-security/nanossl
This is proprietary code and cannot be downloaded and used freely.

After looking at all of them, the winner is: mbed TLS :)

Integrate is with the Bibledit library as follows:
* Copy the current working version of mbedtls to folder mbedtls.old for fallback.
* Download the source tarball and unpack it.
* Copy the unpackaged source library/*.c to folder mbedtls in Bibledit.
* Copy the unpacked source include/mbedtls/*.h to folder mbedtls in Bibledit.
* Put all mbedtls/*.c in Bibledit into Makefile.am in Bibledit.

Example code of SSL/TLS handshake, server, client, and so on, is in the source code of mbed TLS.

To use the library in a threaded environment, define the following in config.h:
MBEDTLS_THREADING_C
MBEDTLS_THREADING_PTHREAD

To disable assembler, undefine the following in config.h:
MBEDTLS_HAVE_ASM
MBEDTLS_AESNI_C
MBEDTLS_PADLOCK_C
The reason that assembler has been disabled is that on Android there were crashes after the mbed TLS library was introduced, and when the internet connection was off. After disabling assembler, the crashes were fewer, although still there. Since disabling assembler led to improved behaviour, it was left disabled.

To have the Bibledit secure http server listen on IPv6 requires a small change, because with the default configuration it will listen on the first address presented by the operating system that succeeds. And that is IPv4 on Linux. According to the specifiations it should present an IPv6 address first. But Ubuntu server does not do that. The change in the source code makes it to look for IPv6 addresses only. Tests indicate that it is safe to do so, because listening on IPv6 will also accept IPv4 connections, even if the network interface has no IPv6 address assigned to it.
In function mbedtls_net_bind in file net_sockets.c, change this line:
  hints.ai_family = AF_UNSPEC;
to this:
  hints.ai_family = AF_INET6;
