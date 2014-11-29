#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "TTTD_s.h"


using std::unordered_map;
using std::string;
using std::vector;


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
  unordered_map<Chunk, int> freqTable;
  int Tmax;
  int Tmin;
  int sampleRate;
  int lookupTable[20];

public:
  FBCChunker(int mx, int mn, int r) {
    srand(time(NULL));
    Tmax = mx;
    Tmin = mn;
    sampleRate = r;
    lookupTable[0] = 1 % sampleRate;
    for (int i = 1; i < 20; i++) {
      lookupTable[i] =  (lookupTable[i - 1] * (256 % sampleRate)) % sampleRate;
    }
  }

  bool prefilter(string fp) {
    int result = 0;
    for (int i = 0; i < 20; i++) {
      result += ((fp[i] % sampleRate) * lookupTable[i]) % sampleRate;
    }
    return result % sampleRate == 1;
  }

  bool lookupCandidate(char *data, int length) {
    for (int i = 0; i < 3; i++) {
      if (!filters[i].lookup(data, length))
	return false;
    }
    return true;
  }

  void insertCandidate(char *data, int length) {
    int iFilter = rand() % 3;
    filters[iFilter].insert(data, length);
  }

  vector<Chunk *> *splitBigChunk(Chunk *c) {
    // Get the whole chunks data
    tuple<char *, int> d = c->getChunkData();
    char *data = std::get<0>(d);
    int length = std::get<1>(d);

    vector<Chunk *> * Chunks = new vector<Chunk *>();
    int windowSize = Tmax;
    int curPos = 0;
    // Scan the big chunk with variable window size    
    while (windowSize >= Tmin) {
      // The window forward one byte each time
      for (curPos = 0; curPos < length - windowSize; curPos++) {
	string fingerprint =  generateFingerprint((unsigned char *)(data + curPos), windowSize);
	// If the window chunk can past the prefilter
	if (prefilter(fingerprint)) {
	  // If the chunk can pass all the bloom filters
	  if (lookupCandidate(data + curPos, windowSize)) {
	    Chunk c(data + curPos, fingerprint, windowSize);
	    // If the chunk is already in frequency table
	    if (freqTable.find(c) != freqTable.end()) {
	      freqTable[c] += 1;
	    } else {
	      // With 3 bloom filters, the expect number of occurence before a chunk add to all 3 bloom filter is 5.5. So the initial value for a chunk in frequent table is 6
	      freqTable[c] = 6;
	    }
	  } else {
	    // The chunk didn't pass all 3 bloom filters, add to one random bloom filter
	    insertCandidate(data + curPos, windowSize);
	  }
	}
      }
      // Each time half the window size
      windowSize = windowSize / 2;
    }
    return Chunks;
  }

};
