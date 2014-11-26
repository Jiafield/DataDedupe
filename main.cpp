#include "Utilities.h"
#include "TTTD_s.h"
#include <iostream>
#include <fstream>

int main(int argc, char *argv[]) {
  std::filebuf fb;
  if (fb.open(argv[1], std::ios::in))
  {
    std::istream input(&fb);
    TTTDsChunker chunker(460, 2800, 540, 270, 1, 1600);
    vector<Chunk *> *chunks = chunker.createChunks(input);
    fb.close();
  }
  return 0;
}
