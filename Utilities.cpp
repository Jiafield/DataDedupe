#include "Utilities.h"
#include "TTTD_s.h"
#include <openssl/sha.h>
#include <sstream>
#include <string>
#include <functional>

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


/* Algorithm : FNV hash
   Reference: http://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function*/
unsigned long long int hash1(const char *data, int len) {
  unsigned long long int hash = 0xcbf29ce484222325;  // This value is default
  unsigned long long int FNV_prime = 0x100000001b3;
  for (int i = 0; i < len; i++) {
    hash = hash ^ data[i];
    hash = hash * FNV_prime;
  }
  return hash;
}

/* Algorithm : Jenkins hash
   Reference: http://en.wikipedia.org/wiki/Jenkins_hash_function */
unsigned long long int hash2(const char *data, int len) {
  unsigned long long int hash;
  int i;
  for(hash = i = 0; i < len; ++i)
    {
      hash += data[i];
      hash += (hash << 10);
      hash ^= (hash >> 6);
    }
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  return hash;
}

/* Algorithm : MurmurHash
   Reference: http://en.wikipedia.org/wiki/MurmurHash */
unsigned long long int hash3(const char *data, int len) {
  static const unsigned int c1 = 0xcc9e2d51;
  static const unsigned int c2 = 0x1b873593;
  static const unsigned int r1 = 15;
  static const unsigned int r2 = 13;
  static const unsigned int m = 5;
  static const unsigned int n = 0xe6546b64;
  
  unsigned int hash = 0x9747b28c;
  
  const int nblocks = len / 4;
  const unsigned int *blocks = (const unsigned int *)data;
  int i;
  for (i = 0; i < nblocks; i++) {
    unsigned int k = blocks[i];
    k *= c1;
    k = (k << r1) | (k >> (32 - r1));
    k *= c2;
    
    hash ^= k;
    hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
  }
  
  const unsigned char *tail = (const unsigned char*)(data + nblocks * 4);
  unsigned int k1 = 0;
  
  switch (len & 3) {
  case 3:
    k1 ^= tail[2] << 16;
  case 2:
    k1 ^= tail[1] << 8;
  case 1:
    k1 ^= tail[0];
    
    k1 *= c1;
    k1 = (k1 << r1) | (k1 >> (32 - r1));
    k1 *= c2;
    hash ^= k1;
  }
  
  hash ^= len;
  hash ^= (hash >> 16);
  hash *= 0x85ebca6b;
  hash ^= (hash >> 13);
  hash *= 0xc2b2ae35;
  hash ^= (hash >> 16);
  
  return hash;
}
