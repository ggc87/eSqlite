#define AESKeyLength 128
#define AESBlockLengthByte 16
#define MAXLengthByte 1024
#define MAXRecs 20

#include "eSqlite3.h"

char *EncodedMACaesEnc(char *Pt, char *key);
char *EncodedMACaesDec(char *EncodedCt, char *key);
int  writeRow(pairing_t *pairing, Asetup_t *SetUp,char *line, int l, FILE *fout, FILE *fkey, FILE *fenckey);
int writeKRow(pairing_t *pairing, Asetup_t *SetUp,char *line, int l, FILE *fout, FILE *fkey, FILE *fenckey);
unsigned char **readRow(pairing_t *pairing, Asetup_t *SetUp,char *line, int l, FILE *fkey);
int writeKRowDb(pairing_t *pairing, Asetup_t *SetUp,char *line,char **cols, int l, eSqlite3 *db,char *tabName, FILE *fkey, FILE *fenckey);
unsigned char **readRow(pairing_t *pairing, Asetup_t *SetUp,char *line, int l, FILE *fkey);

char **parseLine(char *line, int l);
