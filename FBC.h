#ifndef FBC
#define FBC

#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
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
  BloomFilter(unsigned long long int entries);

  BloomFilter();

  void insert(const char *data, int len);

  bool lookup(const char *data, int len);

  bool lookup_one(unsigned long long int result);

  ~BloomFilter();
};


class FBCChunker {
private:
  BloomFilter filters[3];    // The number of bloom filters is default 3, according to the FBC paper, 3 works best in practice.
  unordered_map<Chunk, int, ChunkHash<Chunk>> freqTable;
  int Tmax;
  int Tmin;
  int sampleRate;
  int lookupTable[20];

public:
  FBCChunker(int mx, int mn, int r);

  // A candidate can pass if its fingerprint % sampleRate == 1 
  bool prefilter(string fp);

  // Look up if the candidate chunk is in all the bloom filters
  bool lookupCandidate(char *data, int length);

  // Random choose a bloom filter and add the chunk candidate to the filter
  void insertCandidate(char *data, int length);

  void splitBigChunk(Chunk *c);

};
#endif
