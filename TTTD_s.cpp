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
  Chunk(char *data, FingerprintType fp, int len) {
    dataPtr = (char *)malloc(len * sizeof(char));
    strncpy(dataPtr, data, len);
    fingerprint = fp;
    length = len;
  }

  /* method: getChunkData
     Usage: external representation of the data stream
     return: the dataPtr and the length of the data as a tuple
   */
  tuple<char *, int> getChunkData() {
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
  int stepSize;
  int switchP;
  bool switchStatus;  // true means switched

public:
  // Constructor
  TTTDsChunker(int mini, int maxi, int pD, int sD, int step, int swP) {
    Tmin = mini;
    Tmax = maxi;
    primaryD = pD;
    secondaryD = sD;
    stepSize = step;
    switchP = swP;
    switchStatus = false;

    // Because of the TTTD-s algorithm need to switch the divisor to its half sometimes, check the primaryD and secondaryD must be a multiple of 2
    if (primaryD % 2 || secondaryD % 2) {
      cout << "The primaryD and secondaryD must be a multiple of 2" << endl;
      exit(EXIT_FAILURE);
    }
  }

  void switchDivisor() {
    primaryD = primaryD / 2;
    secondaryD = secondaryD / 2;
    switchStatus = true;
  }

  void resetDivisor() {
    if (switchStatus) {
      primaryD = primaryD * 2;
      secondaryD = secondaryD * 2;
      switchStatus = false;
    }
  }

  /* method: create chunks
     input: data stream
     output: a vector of chunks, each chunk is represented as a tuple of data pointer and length
     algorithm: TTTD-s
   */
  vector<Chunk *> createChunks(istream input) {
    int curLength = 0;
    int backupBreak = 0;

    char *buffer = (char *)malloc(Tmax * sizeof(char));
    char *swapBuffer = (char *)malloc(Tmax * sizeof(char));

    FingerprintType fingerprint;
    FingerprintType backupFingerprint;

    vector<Chunk *> chunks;

    while (!input.eof()){
      // Get the input
      if (curLength == 0) {
	input.read(buffer, Tmin);
      } else {
	input.read(buffer + curLength, stepSize);
      }
      curLength += input.gcount();

      // Update fingerprint
      fingerprint = generateFingerprint(buffer, curLength);

      // if reach the switch paramter, switch divisor
      if (curLength > switchP) {
	if (!switchStatus)
	  switchDivisor();
      }
      // See if we have a backup break point
      if (fingerprint % secondaryD == static_cast<unsigned int>(secondaryD -1)) {
	backupBreak = curLength;
	backupFingerprint = fingerprint;
      }
      // See if we have a break point
      if (fingerprint % primaryD == static_cast<unsigned int>(primaryD - 1)) {
	chunks.push_back(new Chunk(buffer, fingerprint, curLength));
	backupBreak = 0;
	curLength = 0;
	resetDivisor();
	continue;
      }
      // If reach the max length limit
      if (curLength >= Tmax) {
	if (backupBreak != 0) {
	  // If there is a backup point
	  chunks.push_back(new Chunk(buffer, backupFingerprint, backupBreak));
	  // Put the data after the backup break point to swapBuffer
	  strncpy(swapBuffer, buffer + backupBreak, Tmax - backupBreak);
	  char *temp = swapBuffer;
	  swapBuffer = buffer;
	  buffer = temp;
	  resetDivisor();
	  curLength = Tmax - backupBreak;
	  backupBreak = 0;
	} else {
	  // If no backup point, use current length
	  chunks.push_back(new Chunk(buffer, fingerprint, curLength));
	  backupBreak = 0;
	  curLength = 0;
	  resetDivisor();  
	}
      } 
    }
    // If the last chunk before eof in buffer dose not meet all the break creterias, still need to add to the chunks' list
    if (curLength) {
      chunks.push_back(new Chunk(buffer, fingerprint, curLength));
    }
    return chunks;
  }

  // Destructor
  ~TTTDsChunker();
};
