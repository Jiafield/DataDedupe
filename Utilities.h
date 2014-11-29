#ifndef utilities
#define utilities

#include <string>

using std::string;

string generateFingerprint(unsigned char *, int len);

/* Algorithm : FNV hash
   Reference: http://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function*/
unsigned long long int hash1(const char *data, int len);

/* Algorithm : Jenkins hash
   Reference: http://en.wikipedia.org/wiki/Jenkins_hash_function */
unsigned long long int hash2(const char *data, int len);

/* Algorithm : MurmurHash
   Reference: http://en.wikipedia.org/wiki/MurmurHash */
unsigned long long int hash3(const char *data, int len);
 

#endif
