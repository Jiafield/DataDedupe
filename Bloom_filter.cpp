#include <stdlib.h>
#include <time.h>
#include <unordered_map>

using std::unordered_map;

class FreqChunk {
private:
  int ID;    // Store the chunk ID from the result of content defined chunking
  int offset;  // Store the offset relative to the CDC chunk
  char *data;  // The data pointer 
  int length;  // The length of this small chunk

public:
  FreqChunk(int id, int off, char *ptr, int len) {
    ID = id;
    offset = off;
    data = ptr;
    length = len;
  }

  char *getData() {
    return data;
  }

  int getLength() {
    return length;
  }
  
  bool operator==(FreqChunk &c) {
    if (length != c.length)
      return false;
    for (int i = 0; i < length; i++) {
      if (data[i] != (c.data)[i])
	return false;
    }
    return true;
  }
};

class BloomFilter {
private:
  unsigned char *lookupTable;
  int numHashes;
  unsigned long long int numEntries;
  unsigned char masks[8];

public:
  BloomFilter(unsigned long long int entries)
    : numHashes(3), masks({0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80})
  {
    // In the lookup table, each bit is an entry of 0 or 1. 
    // Here the table size is 8 times of the number of input element entries.
    // The number of hash functions is default 3
    lookupTable = (unsigned char *)calloc(entries, sizeof(unsigned char));
    numEntries = 8 * entries;
  }

  BloomFilter()
    : numHashes(3), masks({0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80})
  {
    // Default number of entries is 10000
    // In the lookup table, each bit is an entry of 0 or 1. 
    // Here the table size is 8 times of the number of input element entries.
    // The number of hash functions is default 3
    unsigned long long int entries = 10000;
    lookupTable = (unsigned char *)calloc(entries, sizeof(unsigned char));
    numEntries = 8 * entries;
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

  void insert(const char *data, int len) {
    unsigned long long int result[3];
    result[0] = hash1(data, len);
    result[1] = hash2(data, len);
    result[2] = hash3(data, len);
    for (int i = 0; i < numHashes; i++) {
      int index = (result[i] % numEntries) / 8;
      int iMask = (result[i] % numEntries) % 8;
      lookupTable[index] |= masks[iMask];
    }
  }

  bool lookup(const char *data, int len) {
    unsigned long long int result;
    result = hash1(data, len);
    if (!lookup_one(result))
      return false;
    result = hash2(data, len);
    if (!lookup_one(result))
      return false;
    result = hash3(data, len);
    if (!lookup_one(result))
      return false;
    return true;
  }

  bool lookup_one(unsigned long long int result) {
    int index = (result % numEntries) / 8;
    int iMask = (result % numEntries) % 8;
    if (lookupTable[index] & masks[iMask])
      return true;
    return false;
  }

  ~BloomFilter() {
    free(lookupTable);
  }
};


class FBCChunker {
private:
  BloomFilter filters[3];    // The number of bloom filters is default 3, according to the FBC paper, 3 works best in practice.
  unordered_map<FreqChunk, int> countTable;

public:
  FBCChunker() {
    srand(time(NULL));
  }

  bool lookupCandidate(FreqChunk &chunk) {
    for (int i = 0; i < 3; i++) {
      if (!filters[i].lookup(chunk.getData(), chunk.getLength()))
	return false;
    }
    return true;
  }

  void insertCandidate(FreqChunk &chunk) {
    int iFilter = rand() % 3;
    filters[iFilter].insert(chunk.getData(), chunk.getLength());
  }

  
};
