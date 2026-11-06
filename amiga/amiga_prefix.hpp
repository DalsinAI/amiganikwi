/* Included before every C++ file (-include): the C++ streams first, since
 * the toolchain's pthread.h wraps read(), write() and close() in
 * cancellation macros once <unistd.h> is in, which breaks <istream> and
 * <fstream> read after it. The AmiCradle Team. zlib licence, like
 * Nikwi. */
#include <iostream>
#include <fstream>
#include <sstream>
