#include <iostream>
#include <tuple>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "Utilities.h"

using std::tuple;
using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::istream;

class Chunk {
private:
  char *dataPtr;  // In the simulation, dynamic allocate memory to store the chunk metadata
  long long unsigned fingerprint;
  int length;
public:
  /* Constructor */
  Chunk(char *data, long long unsigned fp, int len) {
    dataPtr = (char *)malloc(len * sizeof(char));
    strncpy(dataPtr, data, len);
    fingerprint = fp;
    length = len;
  }

  /* method: getChunkID
     Usage: external representation of the data stream
     return: the dataPtr and the length of the data as a tuple
   */
  tuple<char *, int> getChunkID() {
    return tuple<char *, int>(dataPtr, length);
  }

  /* Overload == operator 
     Usage: compare if two chunks are same
     Precondition: the fingerprints are already the same
     return true or false
   */
  bool operator==(Chunk &c) {
    if (c.length != length)
      return false;
    for (int i = 0; i < length; i++) {
      if ((c.dataPtr)[i] != dataPtr[i])
	return false;
    }
    return true;
  }

  /* Destructor */
  ~Chunk() {
    free(dataPtr);
  }
};

class TTTDsChunker {
private:
  int Tmin;
  int Tmax;
  int primaryD;
  int secondaryD;
  int remainder;
  int stepSize;
  int switchP;

public:
  // Constructor
  TTTDsChunker(int mini, int maxi, int pD, int sD, int r, int step, int swP) {
    
  }

  /* method: create chunks
     input: data stream
     output: a vector of chunks, each chunk is represented as a tuple of data pointer and length
     algorithm: TTTD-s
   */
  vector<tuple<char *, int>> createChunks(istream input) {
    return NULL;
  }

  // Destructor
  ~TTTDsChunker();
};
