//Encodes Base64
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
 
int Base64Encode(const char* message, char** buffer, int len) { 
//Encodes a string to base64
  BIO *bio, *b64;
  size_t size;
  FILE* stream;
  int encodedSize = 4*ceil((double)len/3);
  *buffer = (char *)malloc(encodedSize+1);
 
  stream = fmemopen(*buffer, encodedSize+1, "w");
  b64 = BIO_new(BIO_f_base64());
  bio = BIO_new_fp(stream, BIO_NOCLOSE);
  bio = BIO_push(b64, bio);
  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); 
  //Ignore newlines - write everything in one line
  BIO_write(bio, message, len);
  BIO_flush(bio);
  BIO_free_all(bio);
  fclose(stream);
 
  return (0); //success
}
