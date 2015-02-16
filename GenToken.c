#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "hve_amm.h"
#include "aesU.h"

char *MasterKeyDir="MasterKeys";
char *TokenDir="Tokens";

int 
main(int argc, char ** argv) 
{

    FILE *ftok,     *fkey,     *fmsk;
    char *ftokName, *fkeyName, *fmskName;


/* print string n2 of all lines in which string n1 is name */
    if (8!=argc){
        fprintf(stderr,"usage: %s <pairing> <mskfile> <tokenfile> <tobematched> <n1> <n2>\n",argv[0]);
        fprintf(stderr,"\tgenerate token to read columns sn2 of all rows in which column n1 is <tobematched>\n");
        fprintf(stderr,"\tuse master key in <mskfile> ");
        fprintf(stderr,"generated using bilinear setting described in file <pairing>\n");
        fprintf(stderr,"\twrite token in file <tokenfile>\n");
        return -1;
    }

    pairing_t *pairing = load_pairing(argv[1]);
#ifdef VERBOSE
    printInfo(*pairing);
#endif
    int n1=atoi(argv[5]);
    int n2=atoi(argv[6]);
    int n3=atoi(argv[7]);

/* open file msk file */
    fmskName=argv[2];
    if ((fmsk=fopen(fmskName,"r"))==NULL){
         fprintf(stderr,"Cannot open file %s for reading\n",fmskName);
         return -1;
    }

/* open token file */
    ftokName=argv[3];
    if ((ftok=fopen(ftokName,"w"))==NULL){
        fprintf(stderr,"Cannot open file %s for writing\n",ftokName);
        return -1;
    }


    Asetup_t * SetUp=readAMsk(pairing,fmsk);       fclose(fmsk);

    int l=SetUp->l;
    if (n1>l){
        fprintf(stderr,"not enough columns in a row (n1=%d>l=%d)\n",n1,l);
        return -1;
    }
    if (n2>l){
        fprintf(stderr,"not enough columns in a row (n2=%d>l=%d)\n",n2,l);
        return -1;
    }

//attributes for reading n2 string of lines with n1 string=argv[4] 
    int *Attr=malloc((l+2)*sizeof(int));
    for(int i=1;i<l+1;i++) Attr[i]=-1;
    Attr[n1]=*((unsigned int *) sha256(argv[4]));
    Attr[n3]=*((unsigned int *) sha256(argv[4]));
    Attr[l+1]=n2;
#ifdef VERBOSE
    for(int i=1;i<l+2;i++) printf("(%d) The attr is %d\n",i,Attr[i]);
#endif

//generate decryption key for Attr
    Akey_t *Key2=Akeygen(pairing,SetUp->privatel,SetUp->private1," prova prova prova prova",Attr);

    storeAkey(pairing,Key2,ftok);
    fclose(ftok);

    return 0;

} 
