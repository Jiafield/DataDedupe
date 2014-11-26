#include "Utilities.h"
#include "TTTD_s.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>

using std::cout;
using std::endl;

int main(int argc, char *argv[]) {
  std::filebuf fb;
  if (fb.open(argv[1], std::ios::in))
  {
    std::istream input(&fb);
    // The following TTTDs parameters are from TTTDs paper
    TTTDsChunker chunker(460, 2800, 540, 270, 1, 1600);
    vector<Chunk *> *chunks = chunker.createChunks(input);

    for (vector<Chunk *>::iterator c = chunks->begin(); c != chunks->end(); c++) {
      delete *c;
    }
    fb.close();
  } else {
    cout << "Can not open input stream" << endl;
    exit(EXIT_FAILURE);
  }
  return 0;
}
