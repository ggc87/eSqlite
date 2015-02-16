#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>


#include <pbc/pbc.h>

#include "utils.h"
#include "hve_amm.h"
#include "aesU.h"


int 
main(int argc, char ** argv) 
{

    int l, len;
    FILE *fin,     *fout,     *fkey,     *fenckey,     *fmpk;
    char *finName, *foutName, *fkeyName, *fenckeyName, *fmpkName;

    char *line=NULL;


    if (6!=argc){
        fprintf(stderr,"usage: %s <pairing> <mpk> <data> <output> <dbname>\n",argv[0]);
        fprintf(stderr,"\tencrypt table found in file <data>\n");
        fprintf(stderr,"\tusing bilinear setting described in file <pairing>\n");
        fprintf(stderr,"\tusing public key in <mpk>\n");
        fprintf(stderr,"\tstore ciphertext in files <output>.hve and <output>.knc\n");
        fprintf(stderr,"\tstore data in dbname\n");
        return -1;
    }

    fmpkName=argv[2];
    finName=argv[3];

    pairing_t *pairing = load_pairing(argv[1]);
#ifdef VERBOSE
    printInfo(*pairing);
#endif

    if ((fmpk=fopen(fmpkName,"r"))==NULL){
         fprintf(stderr,"%s: cannot open file %s\n",argv[0],fmpkName);
         return -1;
    }
    Asetup_t * SetUp=readAMpk(pairing,fmpk);
    fclose(fmpk);

    if ((fin=fopen(finName,"r"))==NULL){
         fprintf(stderr,"%s: cannot open file %s\n",argv[0],finName);
         return -1;
    }

    foutName=malloc(strlen(argv[4])+5);
    sprintf(foutName,"%s.hve",argv[4]);
    if ((fout=fopen(foutName,"w"))==NULL){
         fprintf(stderr,"Cannot open file %s\n",foutName);
         return -1;
    }

    fkeyName=malloc(strlen(argv[4])+5);
    sprintf(fkeyName,"%s.key",argv[4]);
    if ((fkey=fopen(fkeyName,"w"))==NULL){
         fprintf(stderr,"Cannot open file %s\n",fkeyName);
         return -1;
    }

    fenckeyName=malloc(strlen(argv[4])+5);
    sprintf(fenckeyName,"%s.knc",argv[4]);
    if ((fenckey=fopen(fenckeyName,"w"))==NULL){
         fprintf(stderr,"Cannot open file %s\n",fenckeyName);
         return -1;
    }

    len=0;
    while(getline(&line,&len,fin)!=-1){
	len=0;
        writeKRow(pairing,SetUp,line,SetUp->l,fout,fkey,fenckey);
	    
   }
    
    fclose(fout); fclose(fin); fclose(fkey); fclose(fenckey);
    return 0;

} 
