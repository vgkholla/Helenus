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

    static int calculateStringHash(string toBeHashed) {
        int hashID;
        unsigned char digest[SHA_DIGEST_LENGTH];
        unsigned char convertedToBeHashed[toBeHashed.length() + 1];
        int i;
        for(i = 0; i < toBeHashed.length(); i++) {
            convertedToBeHashed[i] = toBeHashed[i];
        }
        convertedToBeHashed[i] = '\0';
        SHA1(convertedToBeHashed, sizeof(convertedToBeHashed), (unsigned char*)&digest);
        hashID = digest[0];
        return hashID;
    }

public:
    Hash(int truncate)
    {
        t = truncate;
    };

    static int calculateNodeHash(string networkID)
    {
        return calculateStringHash(networkID);
    };

    static int calculateKeyHash(string key)
    {
       return calculateStringHash(key);
    };

    static int calculateKeyHash(long int key)
    {
        /*int hashID;
        unsigned char digest[SHA_DIGEST_LENGTH];
        SHA1((unsigned char*)&key, sizeof(key), (unsigned char*)&digest);
        hashID = digest[0];
        return hashID;*/
        cout<<"ERROR!!!!!!!!!!!!!! No one should be coming here!!!!"<<endl;
        return 0;
    };

private:
    int t;

};
#endif
