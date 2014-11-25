#include "Utilities.h"
#include <openssl/sha.h>
#include <sstream>

/* function: generate fingerprint for a chunk
   input: a chunk of data and length of data
   output: long long unsigned int
   algorithm: 
*/
FingerprintType generateFingerprint(unsigned char *data, int len) {
  unsigned char fingerprint[20];
  FingerprintType fp;

  SHA1(data, len, fingerprint);

  std::stringstream ss;
  ss << std::hex << fingerprint;
  ss >> fp;

  return fp;
}

