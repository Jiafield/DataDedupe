#ifndef TTTD_s
#define TTTD_s

#include <tuple>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "Utilities.h"

using std::tuple;
using std::vector;
using std::istream;

class Chunk {
private:
  char *dataPtr;  // In the simulation, dynamic allocate memory to store the chunk metadata
  long long unsigned fingerprint;
  int length;
public:
  /* Constructor */
  Chunk(char *data, FingerprintType fp, int len);


  /* method: getChunkData
     Usage: external representation of the data stream
     return: the dataPtr and the length of the data as a tuple
   */
  tuple<char *, int> getChunkData();


  /* Overload == operator 
     Usage: compare if two chunks are same
     return true or false
   */
  bool operator==(Chunk &c);

  /* Destructor */
  ~Chunk();
};

class TTTDsChunker {
private:
  int Tmin;
  int Tmax;
  int primaryD;
  int secondaryD;
  int stepSize;
  int switchP;
  bool switchStatus;  // true means switched

public:
  // Constructor
  TTTDsChunker(int mini, int maxi, int pD, int sD, int step, int swP);

  void switchDivisor();

  void resetDivisor();


  /* method: create chunks
     input: data stream
     output: a vector of chunks, each chunk is represented as a tuple of data pointer and length
     algorithm: TTTD-s
   */
  vector<Chunk *> createChunks(istream input);

  // Destructor
  ~TTTDsChunker();
};

#endif
