#include <stdlib.h>

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
    // Here the table size is 8 times of the number of element entries.
    // The number of hash function default is 3, don't change it
    lookupTable = (unsigned char *)calloc(entries, sizeof(unsigned char));
    numEntries = 8 * entries;
  }

  unsigned long long int hash1(const void *data, int len) {
    return 0;
  }

  unsigned long long int hash2(const void *data, int len) {
    return 0;
  }

  unsigned long long int hash3(const void *data, int len) {
    return 0;
  }

  void insert(const void *data, int len) {
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

  bool lookup(const void *data, int len) {
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
