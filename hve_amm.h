#ifndef HVE_AMM_H
#define HVE_AMM_H

#include <pbc/pbc.h>

typedef struct {
  unsigned int l;
  element_t ***B; /* l+2 3x3 matrices of elements in Zr + B^{0,*}*/
  element_t gT;
}* mpk_t;

typedef struct {
  unsigned int l;
  element_t ***C; /*l+2 3x3 matrices of elements in Zr + C^{0,*}*/ 
}* msk_t;

typedef struct {
  mpk_t public;
  msk_t private;
  element_t g;
}* setup_t;

typedef struct {
    mpk_t publicl;
    mpk_t public1;
    msk_t privatel;
    msk_t private1;
    int l; /* l attributes are equal and 1 may vary */
} Asetup_t;

typedef struct {
  unsigned int l;
  unsigned int n; /* all vectors start at 1*/
  element_t *c; /* n elements from GT */
  element_t **cs; /* n vectors of 3 elements of G1 */
  element_t **cl; /* n vectors of 3 elements of G1 */
  element_t **ci; /* l+1 vectors of 3 elements of G1 */
} Act_t;

typedef struct {
    unsigned int l;
    element_t c; /* one element in GT */
    element_t **ci; /* l+1 vectors of 2 elements from G1 */
} ct_t;

typedef struct {
  unsigned int l;
  element_t **k; /* l+1 vectors of 3 elements of G2 some maybe NULL */
}* dkey_t;

typedef struct {
  unsigned int l;
  unsigned int n; /* the token allows to decrypt column n*/
  char *cond;     /* condition of the SELECT query */
  element_t **kl; /* l+1 vectors of 3 elements of G2 some maybe NULL */
  element_t **k1; /* 2 vectors of 3 elements of G2 */
} Akey_t;

void v_times_m(element_t *r, element_t *v, element_t **m);

setup_t setup(pairing_t *pairing, element_t psi, element_t g1, element_t g2, int n);
Asetup_t *Asetup(pairing_t *pairing, int n);

ct_t *encrypt(pairing_t *pairing, mpk_t public, int *x, element_t *m);
Act_t *Aencrypt(pairing_t* pairing, mpk_t publicl, mpk_t public1, int *x, int n, element_t *m);

Akey_t  *Akeygen(pairing_t* pairing, msk_t privatel, msk_t private1,char *condition, int *y);
dkey_t keygen(pairing_t *pairing, msk_t private, int *y);

element_t *Adecrypt(pairing_t* pairing, Act_t *ct, Akey_t *key);
element_t *ASingdecrypt(pairing_t *pairing, Act_t *ct, Akey_t *key, int j);
element_t *decrypt(pairing_t *pairing, ct_t *ct, dkey_t key);

void storeSetup(pairing_t *pairing, Asetup_t *key, FILE *fout);
void storeAMpk(pairing_t *pairing, Asetup_t *setUp, FILE *fout);
#define storeAMsk(pairing,setUp,fmsk) storeSetup(pairing,setUp,fmsk)
void storeAct(pairing_t *pairing, Act_t *CT, FILE *fout);

Asetup_t *readSetup(pairing_t *pairing, FILE *fin);
Asetup_t *readAMpk(pairing_t *pairing, FILE *fin);
#define readAMsk(pairing,fin) readSetup(pairing,fin)
Act_t * readAct(pairing_t *pairing,  FILE *fin);

void storeAkey(pairing_t *pairing, Akey_t *token, FILE *fout);
Akey_t *readAkey(pairing_t *pairing, FILE *fin);

char *sha256(char *string);


#endif
