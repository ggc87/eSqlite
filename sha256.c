#include <stdlib.h>
#include <string.h>
#include "openssl/sha.h"

unsigned char *
sha256(char *string)
{
    unsigned char *hash=malloc(SHA256_DIGEST_LENGTH);
    char *hashstr = malloc(SHA256_DIGEST_LENGTH);
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, string, strlen(string));
    SHA256_Final(hash, &sha256);
    hash[SHA256_DIGEST_LENGTH-1]=0;
    return hash;
}

//void sha256Orig(char *string, char outputBuffer[65])
//{
    //unsigned char hash[SHA256_DIGEST_LENGTH];
    //SHA256_CTX sha256;
    //SHA256_Init(&sha256);
    //SHA256_Update(&sha256, string, strlen(string));
    //SHA256_Final(hash, &sha256);
    //int i = 0;
    //for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    //{
        //sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    //}
    //outputBuffer[64] = 0;
//}
