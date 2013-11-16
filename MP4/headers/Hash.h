#ifndef HASH_HEADER
#define HASH_HEADER

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <ctime>
#include <time.h>
#include <openssl/sha.h>

using namespace std;

class Hash
{
public:
    Hash(int truncate)
    {
        t = truncate;
    };

    static int calculateNodeHash(string networkID)
    {
        int hashID;
        unsigned char digest[SHA_DIGEST_LENGTH];
        SHA1((unsigned char*)&networkID, strlen(networkID.c_str()), (unsigned char*)&digest);
        hashID = digest[0];
        return hashID;
    };

    static int calculateKeyHash(long int key)
    {
        int hashID;
        unsigned char digest[SHA_DIGEST_LENGTH];
        SHA1((unsigned char*)&key, sizeof(key), (unsigned char*)&digest);
        hashID = digest[0];
        return hashID;
    };

private:
    int t;

};
#endif
