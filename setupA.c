#include "hve_amm.h"
#include "utils.h"

#include <pbc/pbc.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <math.h>

#include <sqlite3.h>

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}



int 
main(int argc, char ** argv) {
  
    if (5!=argc){
        fprintf(stderr,"usage: %s <pairing> <l> <filename> <dbname>\n",argv[0]);
        fprintf(stderr,"\tgenerate key for table with l columns\n");
        fprintf(stderr,"\tusing bilinear setting described in file <pairing>\n");
        fprintf(stderr,"\tstore public key in <filename>.mpk\n");
        fprintf(stderr,"\tstore secret key in <filename>.msk\n");
        fprintf(stderr,"\tcreate database <dbname>.db\n");
	exit(-1);
    }

    sqlite3 *db;
    char *zErrMsg = 0;
    char *sql;
    int rc;
    printf("%s",argv[4]);
    rc = sqlite3_open(argv[4],&db);
   
    if( rc ){
    	fprintf(stderr,"Can't open database:%s\n",sqlite3_errmsg(db));
	exit(-1);
    }
   
    pairing_t *pairing = load_pairing(argv[1]);
    printInfo(*pairing);


    char *mpkName, *mskName;
    int l=atoi(argv[2]);
    sql = malloc(350*sizeof(sql));
    sprintf(sql,"CREATE TABLE DATA( ID INTEGER PRIMARY KEY," );
    char *str;
    str = malloc(50*sizeof(char));
    for(int i=0;i<l-1;i++){
	sprintf(str,"DATA%d TEXT NOT NULL,",i);
	strcat(sql,str);
    } 
	sprintf(str,"DATA%d TEXT NOT NULL );",l-1);
	strcat(sql,str);
   
    rc = sqlite3_exec(db,sql,callback,0,&zErrMsg); 
    printf("sql=%s\n",sql);
    free(sql);
    if( rc != SQLITE_OK ){
	fprintf(stderr, "SQL error: %s\n", zErrMsg);
	sqlite3_free(zErrMsg);    
    }
    sqlite3_close(db);
    mpkName=malloc(strlen(argv[3])+5);
    sprintf(mpkName,"%s.mpk",argv[3]);
    FILE *fmpk=fopen(mpkName,"w");
    if (fmpk==NULL){
        fprintf(stderr,"Cannot open file %s\n",mpkName);
        return -1;
    }
    
    mskName=malloc(strlen(argv[3])+5);
    sprintf(mskName,"%s.msk",argv[3]);
    FILE *fmsk=fopen(mskName,"w");
    if (fmsk==NULL){
        fprintf(stderr,"Cannot open file %s\n",mskName);
        return -1;
    }

    Asetup_t *setUp=Asetup(pairing,l);
    storeAMpk(pairing,setUp,fmpk);
    storeAMsk(pairing,setUp,fmsk);

    return 0;
}

