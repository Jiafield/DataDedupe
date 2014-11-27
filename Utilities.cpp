#include "Utilities.h"
#include <openssl/sha.h>
#include <sstream>
#include <string>


using std::string;
int modLookupTable[20] = {};


/* function: generate fingerprint for a chunk
   input: a chunk of data and length of data
   output: long long unsigned int
   algorithm: sha1
*/
string generateFingerprint(unsigned char *data, int len) {
  unsigned char fingerprint[20];
  // SHA-1 (Secure Hash Algorithm) is a cryptographic hash function with a 160 bit output.
  SHA1(data, len, fingerprint);
  return string((const char*)fingerprint);
}

