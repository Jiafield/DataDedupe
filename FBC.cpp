#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "TTTD_s.h"
#include "FBC.h"

using std::unordered_map;
using std::string;
using std::vector;


BloomFilter::BloomFilter(unsigned long long int entries)
  : numHashes(3), masks({0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80})
{
  // In the lookup table, each bit is an entry of 0 or 1. 
  // Here the table size is 8 times of the number of input element entries.
  // The number of hash functions is default 3
  lookupTable = (unsigned char *)calloc(entries, sizeof(unsigned char));
  numEntries = 8 * entries;
}

BloomFilter::BloomFilter()
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

void BloomFilter::insert(const char *data, int len) {
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

bool BloomFilter::lookup(const char *data, int len) {
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

bool BloomFilter::lookup_one(unsigned long long int result) {
  int index = (result % numEntries) / 8;
  int iMask = (result % numEntries) % 8;
  if (lookupTable[index] & masks[iMask])
    return true;
  return false;
}

BloomFilter::~BloomFilter() {
  free(lookupTable);
}


FBCChunker::FBCChunker(int mx, int mn, int r) {
    srand(time(NULL));
    Tmax = mx;
    Tmin = mn;
    sampleRate = r;
    lookupTable[0] = 1 % sampleRate;
    for (int i = 1; i < 20; i++) {
      lookupTable[i] =  (lookupTable[i - 1] * (256 % sampleRate)) % sampleRate;
    }
  }

// A candidate can pass if its fingerprint % sampleRate == 1 
bool FBCChunker::prefilter(string fp) {
  int result = 0;
  for (int i = 0; i < 20; i++) {
    result += ((fp[i] % sampleRate) * lookupTable[i]) % sampleRate;
  }
  return result % sampleRate == 1;
}

// Look up if the candidate chunk is in all the bloom filters
bool FBCChunker::lookupCandidate(char *data, int length) {
  for (int i = 0; i < 3; i++) {
    if (!filters[i].lookup(data, length))
      return false;
  }
  return true;
}

// Random choose a bloom filter and add the chunk candidate to the filter
void FBCChunker::insertCandidate(char *data, int length) {
  int iFilter = rand() % 3;
  filters[iFilter].insert(data, length);
}

vector<Chunk *> *FBCChunker::splitBigChunk(Chunk *c) {
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
