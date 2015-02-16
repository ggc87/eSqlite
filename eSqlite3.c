#include "hve_amm.h"
#include "utils.h"
#include "aesU.h"

#include <pbc/pbc.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "eSqlite3.h"

static int internal_callback(void *NotUsed, int argc, char **argv, char **azColName){
  /* int i;
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");*/
   return 0;
}

static int callbackPos(void *data, int argc, char **argv, char **azColName){
	if(argv[0]!=NULL){
		*((int*)data)=atoi(argv[0]);
		return 0;
	}

	return 1;
}

static int getpos(eSqlite3 *db, char *tabname, char *colname){
	char sql[200];
	int rc,pos;
	char *errMsg;

	sprintf(sql,"SELECT POS FROM STRTAB WHERE COLNAME='%s_%s';",tabname,colname);
	rc=sqlite3_exec(db,sql,callbackPos,(void*)&pos,&errMsg);
	if(rc != SQLITE_OK){
		printf("%s\n",errMsg);
		return -1;
	}
	return pos;
}

static void reorderCols(eSqlite3 *db,char *tabName, char **cols, char **vals, int n){

	int *pos,j=0,tmpInt;
	char *tmp,*tmpVal;

	pos = malloc(sizeof(int)*n);
	for(int i=0; i<n ; i++){
		pos[i]=getpos(db,tabName,cols[i])-1;
	}

	while(j<n){
		if(pos[j]==j){
			j++;
			continue;
		}
		tmp=cols[pos[j]];
		tmpVal=vals[pos[j]];
		tmpInt=pos[pos[j]];
		cols[pos[j]]=cols[j];
		vals[pos[j]]=vals[j];
		pos[pos[j]]=pos[j];
		cols[j]=tmp;
		vals[j]=tmpVal;
		pos[j]=tmpInt;

	}

	/*for(int i=0; i<n ; i++){
		printf("%s | ",cols[i]);
	}
	printf("\n");*/

}

int eSqlite3_setUp(
	eSqlite3 *db, 	/*An open db*/
	char *pairingFile, /*Pairing filename*/
	int l, /*columns number*/
	const char *filename) /*keys filename */
{

    pairing_t *pairing = load_pairing(pairingFile);
    printInfo(*pairing);


    char *mpkName, *mskName;
    char *sql;
    char *errmsg=0;

     sql = "CREATE TABLE STRTAB("  \
         "COLNAME TEXT PRIMARY KEY   NOT NULL," \
	 "POS INT NOT NULL"\
         ");";



    sqlite3_exec(db,sql,internal_callback,0,&errmsg);
    mpkName=malloc(strlen(filename)+5);
    sprintf(mpkName,"%s.mpk",filename);
    FILE *fmpk=fopen(mpkName,"w");
    if (fmpk==NULL){
        fprintf(stderr,"Cannot open file %s\n",mpkName);
        return -1;
    }

    mskName=malloc(strlen(filename)+5);
    sprintf(mskName,"%s.msk",filename);
    FILE *fmsk=fopen(mskName,"w");
    if (fmsk==NULL){
        fprintf(stderr,"Cannot open file %s\n",mskName);
        return -1;
    }

    Asetup_t *setUp=Asetup(pairing,l);
    storeAMpk(pairing,setUp,fmpk);
    storeAMsk(pairing,setUp,fmsk);

    return 0;
}//end setUp


int eSqlite3_selectGenToken(
	eSqlite3 *db,
	char *pairingFile,	/*Pairing filename */
	const char *fmskstr,	/* Master secret key filename */
	const char *tok,	/* Token filename */
	const char *sql		/* SQL select query */
	)
{

    FILE *ftok,     *fkey,     *fmsk;
    char *ftokName, *fkeyName, *fmskName,*conditions,*tabName,**cols,**vals;
    int *colsPos,n;

    cols = malloc(sizeof(char*));
    vals = malloc(sizeof(char*));

/* print string n2 of all lines in which string n1 is name */


    pairing_t *pairing = load_pairing(pairingFile);
#ifdef VERBOSE
    printInfo(*pairing);
#endif
    extractDataSql(sql,&tabName,&cols,&vals,NULL,&n);
    colsPos = malloc(sizeof(int)*n);
    conditions=malloc(sizeof(char));
    sprintf(conditions,"");
    colsPos[0]=getpos(db,tabName,cols[0]);
    for(int i = 1;i < n ; i++){
	colsPos[i]=getpos(db,tabName,cols[i]);
	conditions = realloc(conditions,sizeof(char)*strlen(cols[i])+strlen(vals[i-1])+1);
	strcat(conditions,cols[i]);//not clever
	strcat(conditions," ");
	strcat(conditions,vals[i-1]);
	strcat(conditions," ");
    }
  /* open file msk file */
    if ((fmsk=fopen(fmskstr,"r"))==NULL){
         fprintf(stderr,"Cannot open file %s for reading\n",fmskstr);
         return -1;
    }

/* open token file */
    if ((ftok=fopen(tok,"w"))==NULL){
        fprintf(stderr,"Cannot open file %s for writing\n",ftokName);
        return -1;
    }


    Asetup_t * SetUp=readAMsk(pairing,fmsk);       fclose(fmsk);

    int l=SetUp->l;
    //attributes for reading n2 string of lines with n1 string=argv[4]
    int *Attr=malloc((l+2)*sizeof(int));
    for(int i=1;i<l+1;i++) Attr[i]=-1;
    for(int j=1;j<n;j++){
	Attr[colsPos[j]]=*((unsigned int*) sha256(vals[j-1]));
	//Attr[l+i]=colsPos[i];
    }
    Attr[l+1]=colsPos[0];
#ifdef VERBOSE
    for(int i=1;i<l+2;i++) printf("(%d) The attr is %d\n",i,Attr[i]);
#endif

//generate decryption key for Attr
    Akey_t *Key2=Akeygen(pairing,SetUp->privatel,SetUp->private1,conditions,Attr);

    storeAkey(pairing,Key2,ftok);
    fclose(ftok);

    free(conditions);
    free(colsPos);
    return 0;
}

int eSqlite3_exec(
  sqlite3* db,                                  /* An open database */
  const char *sql,                           /* SQL to be evaluated */
  int (*callback)(void*,int,char**,char**),  /* Callback function */
  void *arg,                                    /* 1st argument to callback */
  char **errmsg,                             /* Error msg written here */
  char *pairing,			     /* Pairing filename */
  char *mpk, 				     /* Masert public key filename */
  Akey_t **tokens,
  int size
)
{
	char *sqlTok,*tmp;
	tmp=malloc(sizeof(char)*strlen(sql));
	strcpy(tmp,sql);
	int rc = SQLITE_OK;
	sqlTok=strtok(tmp,";");
	while(sqlTok!=NULL){
		char *sqlAct = malloc(sizeof(char)*strlen(sqlTok)+2);
		sprintf(sqlAct,"%s;",sqlTok);
		if(strncmp(sqlAct,"CREATE",6)==0){
			rc = eSqlite3_create(db,sqlAct,internal_callback,0,errmsg);
			if( rc != SQLITE_OK ){
				free(sqlAct);
				goto EXIT_EXEC;			
			}
		}

		if(strncmp(sqlAct,"INSERT",6)==0){
			assert(mpk!=NULL);
			assert(pairing!=NULL);
			rc = eSqlite3_encrypt(db,pairing,mpk,sqlAct,errmsg);
			if( rc != SQLITE_OK ){
				free(sqlAct);
				goto EXIT_EXEC;	
			}
		}

		if(strncmp(sqlAct,"SELECT",6)==0){
			rc = eSqlite3_select(db,sqlAct,pairing,tokens,callback,size);
		}
		free(sqlAct);
		sqlTok=strtok(NULL,";");
	}

EXIT_EXEC:
	free(tmp);
	return rc;
}

int eSqlite3_create(
		eSqlite3* db,                                  /* An open database */
  		const char *sql,                           /* SQL to be evaluated */
  		int (*callback)(void*,int,char**,char**),  /* Callback function */
  		void *arg,                                    /* 1st argument to callback */
  		char **errmsg                             /* Error msg written here */
		)
{
	char *tabName,*foutName;
	char **cols = malloc(sizeof(char*));
	char **vals = malloc(sizeof(char*));
	int n,rc;
	FILE *fout;
	extractDataSql(sql,&tabName,&cols,&vals,NULL,&n);
	foutName=malloc(strlen(tabName)+5);
    	sprintf(foutName,"%s.hve",tabName);

   	 if((fout=fopen(foutName,"a"))==NULL){
		char *msg = "ERROR CREATING TABLE";
		*errmsg=msg;
		return SQLITE_ERROR;
	}

	char *intsql = malloc(sizeof(char)*strlen(sql));
	sprintf(intsql,"CREATE TABLE %s_CRPT( ID TEXT PRIMARY KEY NOT NULL );",tabName);
	rc = sqlite3_exec(db,intsql,internal_callback,0,errmsg);
	if( rc != SQLITE_OK ){
		goto EXITCREATE;

	}
	for(int j=0;j<n;j++){
		sprintf(intsql,"INSERT INTO STRTAB (COLNAME,POS) VALUES ('%s_%s',%d);",tabName,cols[j],j+1);
		rc=sqlite3_exec(db,intsql,internal_callback,0,errmsg);
		if( rc != SQLITE_OK ){
			goto EXITCREATE;
	  	}
	}

	/*rc = sqlite3_exec(db,sql,callback,arg,errmsg);
	if( rc != SQLITE_OK ){
		goto EXITCREATE;
	}*/ //store all data into sqlite database
EXITCREATE: //exit actions
	free(tabName);
	free(intsql);
	free(vals);
	return rc;
}


int eSqlite3_encrypt(
	eSqlite3 *db,		/*An open database*/
	char *pairing,		/*Pairing filename */
	char *mpk,		/*Master public key filename*/
	char *sql,		/*sql insert command*/
  	char **errmsg          /* Error msg written here */
	)
{
	int rc;
	char *tabName, *foutNamer, *fkeyName,*fmpkName, *fenckeyName;
	char **cols = malloc(sizeof(char*));
	char **vals = malloc(sizeof(char*));
	char *sqlInt;
	unsigned char *hash;
	int n,primary;
	FILE *fmpk,*fout,*fkey,*fenckey,*foutName;
	pairing_t *pairingstr = load_pairing(pairing);
#ifdef VERBOSE
    printInfo(*pairingstr);
    printf("sql=%s",sql);
#endif

    extractDataSql(sql,&tabName,&cols,&vals,&primary,&n);
    fmpkName=malloc(strlen(mpk)+5);
    sprintf(fmpkName,"%s.mpk",mpk);
    if ((fmpk=fopen(fmpkName,"r"))==NULL){
         fprintf(stderr,"%s: cannot open file %s\n",fmpkName,mpk);
         return -1;
    }
    Asetup_t * SetUp=readAMpk(pairingstr,fmpk);
    fclose(fmpk);

    foutName=malloc(strlen(tabName)+5);
    sprintf(foutName,"%s.hve",tabName);

    if((fout=fopen(foutName,"r"))==NULL){
	char *msg = "TABLE DOESNT EXIST";
	*errmsg=msg;
	return SQLITE_ERROR;
    }else{ fclose(fout); }
    if ((fout=fopen(foutName,"a+"))==NULL){
         fprintf(stderr,"Cannot open file %s\n",foutName);
         return -1;
    }

    fkeyName=malloc(strlen(tabName)+5);
    sprintf(fkeyName,"%s.key",tabName);
    if ((fkey=fopen(fkeyName,"a+"))==NULL){
         fprintf(stderr,"Cannot open file %s\n",fkeyName);
         return -1;
    }

    fenckeyName=malloc(strlen(tabName)+5);
    sprintf(fenckeyName,"%s.knc",tabName);
    if ((fenckey=fopen(fenckeyName,"a+"))==NULL){
         fprintf(stderr,"Cannot open file %s\n",fenckeyName);
         return -1;
    }



   char *line;
   line=malloc(sizeof(char));
   sprintf(line,"");
   reorderCols(db,tabName,cols,vals,n);
   hash = sha256(vals[primary]);
   sqlInt = malloc(sizeof(char)*strlen(tabName)+strlen(hash)+36);
   sprintf(sqlInt,"INSERT INTO %s_CRPT(ID) VALUES ('%s');",tabName,hash);
   rc = sqlite3_exec(db,sqlInt,internal_callback,0,errmsg);
   if(rc != SQLITE_OK ){
	goto EXIT_INSERT;
   }
   for(int i=0;i<n;i++){
   	line=realloc(line,strlen(line)+strlen(vals[i])+2);
	strcat(line,vals[i]);
	strcat(line," ");
   }

   if(SetUp->l!=n){
 	*errmsg = malloc(sizeof(char)*50);
	sprintf(*errmsg,"Invalid number of column");
	rc = SQLITE_ERROR;
	goto EXIT_INSERT;
   }
   //rc = writeKRowDb(pairingstr,SetUp,line,cols,SetUp->l,db,tabName,fkey,fenckey);
   writeKRow(pairingstr,SetUp,line,SetUp->l,fout,fkey,fenckey);
EXIT_INSERT:
   free(fmpkName);
   free(fenckeyName);
   free(fkeyName);
   free(hash);
   free(sqlInt);
   return rc;
}


int eSqlite3_addToken(
	char *pairingstr, 	/* pairing filename */
	char **tokensFile,      /*list of tokens filename*/
	Akey_t ***tokens,  	/*list of open token*/
	int n,		  	/*number of open token*/
	int size         	/*size of tokens filename list*/
	)
{
	pairing_t *pairing = load_pairing(pairingstr);
	Akey_t **toReturn;
	
	if(!n){
		*tokens = malloc(sizeof(Akey_t*)*size);
	}
	else
		*tokens = realloc(*tokens,sizeof(Akey_t*)*(size+n+1));

	for(int i = 0; i < size; i++){
		FILE *fp;
		if ((fp=fopen(tokensFile[i],"r"))==NULL){
        		fprintf(stderr,"Cannot open file %s\n",tokensFile);
        		return -1;
		}

    		(*tokens)[i+n]=readAkey(pairing, fp);
		fclose(fp);
	}
//	*tokens=toReturn;
	return size+n;
}

void setP(char a){
	char p=a;
}


int eSqlite3_select(
		eSqlite3 *db,
		char *sql,
        	char *pairingFile,
		Akey_t **tokens,
  		int (*callback)(void*,int,char**,char**),  /* Callback function */
		int size
		)
{
	char *tabName,**cols,**vals, *fkeyName, *finName,*check, **results,**azColName;
	FILE *fkey, *fin;
	int posVal,n,posToDecrypt,argc;
	char *line=NULL;

        pairing_t *pairing = load_pairing(pairingFile);

	results=malloc(sizeof(char*));
	azColName=malloc(sizeof(char*));
/*	for(int k=0;k<10;k++)
		results = malloc(sizeof(char)*100);*/
	cols = malloc(sizeof(char*));
	vals = malloc(sizeof(char*));
	check = malloc(sizeof(char));
	
	sprintf(check,"");
	extractDataSql(sql,&tabName,&cols,&vals,&posVal,&n);
	finName=malloc(strlen(tabName)+5);
	fkeyName=malloc(strlen(tabName)+5);
	sprintf(finName,"%s.hve",tabName);
        if ((fin=fopen(finName,"r"))==NULL){
             fprintf(stderr,"%s: cannot open file %s\n",tabName,finName);
             return -1;
        }

        sprintf(fkeyName,"%s.knc",tabName);
        if ((fkey=fopen(fkeyName,"r"))==NULL){
             fprintf(stderr,"%s: cannot open file %s\n",tabName,fkeyName);
             return -1;
        }
	free(finName);
	free(fkeyName);
	
	for(int i=posVal;i < n; i++){
			check = realloc(check,sizeof(char)*strlen(cols[i])+strlen(vals[i-posVal])+2);
			strcat(check,cols[i]);
			strcat(check," ");
			strcat(check,vals[i-posVal]);
			strcat(check," ");
		}

		strcat(check,"\n");
	
        argc = 0;
	for(int j=0;j<posVal;j++){ //it could be better 
		posToDecrypt = getpos(db,tabName,cols[j]);
		char *colName;	
		for( int i = 0;i < size; i++ ){
			if((tokens[i]->n==posToDecrypt) && (strcmp(tokens[i]->cond,check)==0))
			{
			//decrypt
				
				colName = malloc(sizeof(char)*strlen(cols[j]));
				strcpy(colName,cols[j]);
		    		int l=tokens[i]->l;
		    		int n2=tokens[i]->n;

		     
				//read file of Ciphertexts
		   		size_t len=0;
		   		int linenumber=1;
		   		while (getline(&line,&len,fin)!=-1){

		    		//read l encrypted AES keys  corresponding to the line
			    		Act_t *EncAESKeys=readAct(pairing,fkey);

	    			//decrypt using Token and obtain AES key for the n2-th string  of the line
			    		element_t *mOut=ASingdecrypt(pairing,EncAESKeys,tokens[i],n2);
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
			    		if (Pt!=NULL)
						//printf("nothing found in linenumber %d\n",linenumber);
			    		{
						azColName = realloc(azColName,sizeof(char*)*(argc+1));
						azColName[argc] = colName;
						results = realloc(results,sizeof(char*)*(argc+1));
						results[argc] = malloc(sizeof(char)*strlen(Pt)+1);
						strcpy(results[argc],Pt);	
						//argv[argc-1] = Pt;	
			 			argc++;
			
					#ifdef VERBOSE
						element_printf("The random element\n%B\n",*mOut);
						printf("The raw key    ");
						for(int k=0;i<AESKeyLength/8;k++) printf(" %02X",AESKey[k]);
						printf("\n");
					#endif
			    		}
		       			#ifdef VERBOSE
						printf("\n");
					#endif

					linenumber++;
					free(ExtendedKey);
					free(AESKey);
					free(line);
					len=0;
		    		}
		    		fseek(fin,SEEK_SET,0);
				fseek(fkey,SEEK_SET,0);
	    		}
		}
	}
	callback(NULL,argc,results,azColName);
	fclose(fin); fclose(fkey);
	return SQLITE_OK;
}


