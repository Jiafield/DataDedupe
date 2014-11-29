#include "Utilities.h"
#include "TTTD_s.h"
#include "FBC.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>

using std::cout;
using std::endl;

int main(int argc, char *argv[]) {

  // Test TTTDs
  std::filebuf fb;
  if (fb.open(argv[1], std::ios::in))
  {
    std::istream input(&fb);
    // The following TTTDs parameters are from TTTDs paper, in practice we should set the chunk size larger
    TTTDsChunker chunker(460, 2800, 540, 270, 1, 1600);
    vector<Chunk *> *chunks = chunker.createChunks(input);


    // Test FBC
    FBCChunker fChunker(2000, 500, 32);

    for (vector<Chunk *>::iterator c = chunks->begin(); c != chunks->end(); c++) {
      fChunker.splitBigChunk(**c);
    }

    fChunker.printFreqTable();

    fb.close();
  } else {
    cout << "Can not open input stream" << endl;
    exit(EXIT_FAILURE);
  }

  // Test bloom filter
  BloomFilter bf;  
  cout << "Before insert: " << bf.lookup("hello", 6) << endl;
  bf.insert("hello", 6);
  cout << "After insert: " << bf.lookup("hello", 6) << endl;  

  return 0;
}
