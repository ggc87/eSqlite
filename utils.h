#ifndef UTILS_H
#define UTILS_H

#include <pbc/pbc.h>
#include <linux/types.h>

pairing_t *load_pairing(char *params_path);
element_t **transpose(element_t **m, int n);
void minorX(element_t * tmp1, element_t a, element_t b, element_t c, element_t d);
void v_times_m(element_t *r, element_t *v, element_t **m);
void printInfo(pairing_t pairing);

int Base64Decode(char*, char** );
int calcDecodeLength(const char*);
int Base64Encode(const char* message, char** buffer, int len);


#endif
