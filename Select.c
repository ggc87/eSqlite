#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "hve_amm.h"
#include "aesU.h"

char *TokenDir="Tokens";

int 
main(int argc, char ** argv) 
{

    FILE *fkey,     *fin,     *fp;
    char *fkeyName, *finName, *ftokName;


    if (4!=argc){
        fprintf(stderr,"usage: %s <pairing> <tokenfile> <data>\n",argv[0]);
        fprintf(stderr,"\tapply token in file <tokenfile>\n",argv[0]);
        fprintf(stderr,"\tto encrypted table stored in files <data>.hve <data>.knc\n",argv[0]);
        fprintf(stderr,"\ttoken must be generated with master secret key corresponding\n");
        fprintf(stderr,"\tto the master public key used to encrypt data\n");
        fprintf(stderr,"\tgenerated using bilinear setting described in file <pairing>\n");
        
        
        return -1;
    }

    ftokName=argv[2]; 
    pairing_t *pairing = load_pairing(argv[1]);
#ifdef VERBOSE
    printInfo(*pairing);
#endif

    if ((fp=fopen(ftokName,"r"))==NULL){
        fprintf(stderr,"Cannot open file %s\n",ftokName);
        return -1;
    }

    finName=malloc(strlen(argv[3])+5);
    sprintf(finName,"%s.hve",argv[3]);
    if ((fin=fopen(finName,"r"))==NULL){
         fprintf(stderr,"%s: cannot open file %s\n",argv[0],finName);
         return -1; 
    }   

    fkeyName=malloc(strlen(argv[3])+5);
    sprintf(fkeyName,"%s.knc",argv[3]);
    if ((fkey=fopen(fkeyName,"r"))==NULL){
         fprintf(stderr,"%s: cannot open file %s\n",argv[0],fkeyName);
         return -1; 
    }   
//read token
    Akey_t *token=readAkey(pairing, fp);
    int l=token->l;
    int n2=token->n;


//read file of Ciphertexts
    char *line=NULL;
    size_t len=0;
    int linenumber=1;
    while (getline(&line,&len,fin)!=-1){

//read l encrypted AES keys  corresponding to the line
        Act_t *EncAESKeys=readAct(pairing,fkey);

//decrypt using Token and obtain AES key for the n2-th string  of the line
        element_t *mOut=ASingdecrypt(pairing,EncAESKeys,token,n2);
        char *ExtendedKey=malloc(MAXLengthByte);
        element_to_bytes(ExtendedKey,*mOut);
        unsigned char *AESKey=malloc(AESKeyLength/8);
        memcpy(AESKey,ExtendedKey+1,AESKeyLength/8);
 
//get the ciphertexts 
        char **AESCt=parseLine(line,l);
//and decrypt the n2-th ciphertext 
        char *Pt=EncodedMACaesDec(AESCt[n2],AESKey);

#ifdef VERBOSE
        printf("The ciphertext %s\n",AESCt[n2]);
#endif
        if (Pt==NULL)
            printf("nothing found in linenumber %d\n",linenumber);
        else {
            printf("smthing found in linenumber %d\n",linenumber);
            printf("The plaintext: %s\n",Pt);
#ifdef VERBOSE
        element_printf("The random element\n%B\n",*mOut);
        printf("The raw key    ");
        for(int i=0;i<AESKeyLength/8;i++) printf(" %02X",AESKey[i]);
        printf("\n");
#endif
        }
#ifdef VERBOSE
        printf("\n");
#endif
    
    //    if(line) free(line);
    //    line=NULL; 
        linenumber++;
	free(line);
	line=NULL;
	len=0;
    }

    fclose(fin); fclose(fkey);
    return 0;

} 
