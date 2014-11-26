#include <iostream>
#include <tuple>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include "Utilities.h"
#include "TTTD_s.h"

using std::tuple;
using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::istream;
using std::string;

/* Constructor */
Chunk::Chunk(char *data, string fp, int len) {
  dataPtr = (char *)malloc(len * sizeof(char));
  strncpy(dataPtr, data, len);
  fingerprint = fp;
  length = len;
}

/* method: getChunkData
   Usage: external representation of the data stream
   return: the dataPtr and the length of the data as a tuple
*/
tuple<char *, int> Chunk::getChunkData() {
  return tuple<char *, int>(dataPtr, length);
}

/* Overload == operator 
   Usage: compare if two chunks are same
   return true or false
*/
bool Chunk::operator==(Chunk &c) {
  if (length != c.length)
    return false;
  if (fingerprint != c.fingerprint)
    return false;
  for (int i = 0; i < length; i++) {
    if ((c.dataPtr)[i] != dataPtr[i])
      return false;
  }
  return true;
}

/* Destructor */
Chunk:: ~Chunk() {
  free(dataPtr);
}


// Constructor
TTTDsChunker::TTTDsChunker(int mini, int maxi, int pD, int sD, int step, int swP) {
  Tmin = mini;
  Tmax = maxi;
  primaryD = pD;
  secondaryD = sD;
  stepSize = step;
  switchP = swP;
  switchStatus = false;
  
  // The result of finger print is 20 bytes, need a lookup table to avoid overflow during calculating the modulo result. The value stored in the lookup table[i] is (256^i) % divisor.
  primaryLookupTable[0] = 1 % primaryD;
  secondaryLookupTable[0] = 1 % secondaryD;  
  for (int i = 1; i < 20; i++) {
    primaryLookupTable[i] =  (primaryLookupTable[i - 1] * (256 % primaryD)) % primaryD;
    secondaryLookupTable[i] = (secondaryLookupTable[i - 1] * (256 % secondaryD)) % secondaryD;
  }

  // Because of the TTTD-s algorithm need to switch the divisor to its half sometimes, check the primaryD and secondaryD must be a multiple of 2
  if (primaryD % 2 || secondaryD % 2) {
    cout << "The primary divisor and secondary divisor must be a multiple of 2" << endl;
    exit(EXIT_FAILURE);
  }
}

void TTTDsChunker::switchDivisor() {
  primaryD = primaryD / 2;
  secondaryD = secondaryD / 2;
  switchStatus = true;
}

void TTTDsChunker::resetDivisor() {
  if (switchStatus) {
    primaryD = primaryD * 2;
    secondaryD = secondaryD * 2;
    switchStatus = false;
  }
}

bool TTTDsChunker::isBreakPoint(string &fp) {
  int result = 0;
  for (int i = 0; i < 20; i++) {
    result += ((fp[i] % primaryD) * primaryLookupTable[i]) % primaryD;
  }
  return result % primaryD == primaryD - 1;
}

bool TTTDsChunker::isBackupPoint(string &fp) {
  int result = 0;
  for (int i = 0; i < 20; i++) {
    result += ((fp[i] % secondaryD) * secondaryLookupTable[i]) % secondaryD;
  }
  return result % secondaryD == secondaryD - 1;
}

/* method: create chunks
   input: data stream
   output: a vector of chunks, each chunk is represented as a tuple of data pointer and length
   algorithm: TTTD-s
*/
vector<Chunk *> *TTTDsChunker::createChunks(istream &input) {
  int curLength = 0;
  int backupBreak = 0;
  
  char *buffer = (char *)malloc(Tmax * sizeof(char));
  char *swapBuffer = (char *)malloc(Tmax * sizeof(char));
  
  string fingerprint;
  string backupFingerprint;
  
  vector<Chunk *> *chunks = new vector<Chunk *>();

  while (!input.eof()){
    // Get the input
    if (curLength == 0) {
      input.read(buffer, Tmin);
    } else {
      input.read(buffer + curLength, stepSize);
    }
    curLength += input.gcount();
    
    // Update fingerprint
    fingerprint = generateFingerprint((unsigned char *)buffer, curLength);
    
    // if reach the switch paramter, switch divisor
    if (curLength > switchP) {
      if (!switchStatus)
	switchDivisor();
    }
    // See if we have a backup break point
    if (isBackupPoint(fingerprint)) {
      backupBreak = curLength;
      backupFingerprint = fingerprint;
    }
    // See if we have a break point
    if (isBreakPoint(fingerprint)) {
      chunks->push_back(new Chunk(buffer, fingerprint, curLength));
      cout << "PRIMARY " << curLength << endl;
      backupBreak = 0;
      curLength = 0;
      resetDivisor();
      continue;
    }
    // If reach the max length limit
    if (curLength >= Tmax) {
      if (backupBreak != 0) {
	// If there is a backup point
	chunks->push_back(new Chunk(buffer, backupFingerprint, backupBreak));
	cout << "Secondary " << backupBreak << endl;
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
	chunks->push_back(new Chunk(buffer, fingerprint, curLength));
	cout << "MaxSize " << chunks->size() << endl;
	backupBreak = 0;
	curLength = 0;
	resetDivisor();  
      }
    } 
  }
  // If the last chunk before eof in buffer dose not meet any the break creteria, we still need to add it to the chunks' list
  if (curLength) {
    chunks->push_back(new Chunk(buffer, fingerprint, curLength));
    cout << "LastPiece " << curLength << endl;
  }
  return chunks;
}

// Destructor
TTTDsChunker::~TTTDsChunker() {}
