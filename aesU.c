#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <openssl/aes.h>
#include "hve_amm.h"
#include "aesU.h"
#include "utils.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}


char *
EncodedMACaesEnc(char *Pt, char *key)
{

            int lenCt;
            int lenPt=strlen(Pt);
            int lenPadPt=lenPt+5;

            AES_KEY enc_key;
            char ivec[AESBlockLengthByte];
            char *base64EncodedCt;

            if (lenPadPt%AESBlockLengthByte==0)
                lenCt=lenPadPt;
            else
                lenCt=((lenPadPt/AESBlockLengthByte)+1)*AESBlockLengthByte;


            char *PadPt=malloc(lenCt+1);
            memset(PadPt,0x0,lenCt+1);
            PadPt[0]='A'; PadPt[1]='B';
            PadPt[2]='C'; PadPt[3]='D'; PadPt[4]='E';
            memcpy(PadPt+5,Pt,lenPt);
            //memset(PadPt+(lenPt+5),0x0,lenCt-lenPt-5+1);
            //printf("lenCt=%d lenPt=%d strlen=%d\n",lenCt,lenPt,strlen(PadPt));
            //printf("PadPt=%s\n",PadPt);
            //printf("   Pt=%s\n",Pt);

//            printf("   Pt(%d)=%s\nPadPt(%d)=%s\n",strlen(Pt),Pt,strlen(PadPt),PadPt);

            AES_set_encrypt_key(key,AESKeyLength,&enc_key);
            memset(ivec,0x0,AESBlockLengthByte);
            unsigned char *enc_out=malloc(lenCt*sizeof(char));

            AES_cbc_encrypt(PadPt,enc_out,lenCt,&enc_key,ivec,AES_ENCRYPT);
            Base64Encode((const unsigned char *)enc_out, (char **)&base64EncodedCt,lenCt);
            return base64EncodedCt;
}

char *
EncodedMACaesDec(char *EncodedCt, char *key)
{

        AES_KEY dec_key;
        char ivec[AESBlockLengthByte];
        char *ct;

        //printf("Encoded Ct:\n%s\n",EncodedCt);
        int lenCt=calcDecodeLength((const char*)EncodedCt);
        if (lenCt>MAXLengthByte){
            fprintf(stderr,"Cyphertext too long: %d\n",lenCt);
            return NULL;
        }
        Base64Decode(EncodedCt,(char **) &ct); /* we have an AES ciphertext */
        char *dec_out=malloc(MAXLengthByte*sizeof(char));
        memset(dec_out,0x0,MAXLengthByte);
        AES_set_decrypt_key(key,AESKeyLength,&dec_key);
        memset(ivec,0x0,AESBlockLengthByte);
        AES_cbc_encrypt(ct,dec_out,lenCt,&dec_key,ivec,AES_DECRYPT);

        if(*(dec_out+0)!='A') return NULL;
        if(*(dec_out+1)!='B') return NULL;
        if(*(dec_out+2)!='C') return NULL;
        if(*(dec_out+3)!='D') return NULL;
        if(*(dec_out+4)!='E') return NULL;
        return dec_out+5;
}


char **
parseLine(char *line, int l)
{

    int tokennum, lentok;
    char *token;
    char **result=malloc((l+1)*sizeof(char *));

    for(tokennum=1, token=strtok(line," "); token; tokennum++, token=strtok(NULL," ")){
        if (tokennum>l){
            fprintf(stderr,"Too many records (%d) in %s\n",tokennum,line);
            return NULL;
        }
        if (strlen(token)>MAXLengthByte){
            fprintf(stderr,"Record too long: %s\n",token);
            return NULL;
        }
        /* remove \n from last record of the row */
        lentok=strlen(token);
        if (token[lentok-1]=='\n') token[lentok-1]='\0';
        result[tokennum]=malloc((lentok+1)*sizeof(char));
        strcpy(result[tokennum],token);
   }
   tokennum--;
   if (tokennum<l){
        fprintf(stderr,"Too few records (%d)\n",tokennum);
        return NULL;
   }

   return result;
}

int
 writeKRowDb(pairing_t *pairing, Asetup_t *SetUp,char *line,char **cols, int l, eSqlite3 *db, char *tabName, FILE *fkey, FILE *fenckey){

	int rc;
	/*rc = sqlite3_open(dbName,&db);
	if( rc ){
		fprintf(stderr,"Can't open database \n");
		exit(-1);
	}*/
	element_t *RowElements=malloc((l+1)*sizeof(element_t));
        char **AESPt=malloc((l+1)*sizeof(char *));
        unsigned char **AESKeys=malloc((l+1)*sizeof(unsigned char **));
        int lentok, tokennum;
        char *token, *extendedKey, *base64EncodedKey, *base64EncodedCt;
        unsigned int *Attr;

        AESPt=parseLine(line,l);
        if (AESPt==NULL) return -1;
        char **hashAESPt=malloc((l+1)*sizeof(char *));
        for(int i=1;i<l+1;i++)
            hashAESPt[i]=sha256(AESPt[i]);
        Attr=malloc((2*l+2)*sizeof(int));
        for(int i=1;i<l+1;i++){
                Attr[i]=*((int *)hashAESPt[i]);
                Attr[l+1+i]=i;
        }

#ifdef VERBOSE
        for(int i=1;i<l+1;i++){
            printf("Plaintext %s\n",AESPt[i]);
            printf("Attr      %u\n",Attr[i]);
        }
#endif

        char *sql;
	char *zErrMsg = 0;
	sql = malloc(800*sizeof(char));
	sprintf(sql,"INSERT INTO %s(",tabName);
	char str[200];
	for (int i=0;i<l-1;i++){
		sprintf(str,"%s,",cols[i]);
		strcat(sql,str);
	}
		sprintf(str,"%s) VALUES (",cols[l-1]);
		strcat(sql,str);

        for (int i=1;i<l+1;i++){
            /* let us pick a key to encrypt AESPt[i] */
#ifdef VERBOSE
            printf("Column=%d\n",i);
#endif
            element_init_GT(RowElements[i],*pairing);
            element_random(RowElements[i]);
            extendedKey=malloc(MAXLengthByte);
            element_to_bytes(extendedKey,RowElements[i]);
            AESKeys[i]=malloc(AESKeyLength/8*sizeof(unsigned char));
            memcpy(AESKeys[i],extendedKey+1,AESKeyLength/8);
            Base64Encode((const unsigned char *)AESKeys[i], (char **) &base64EncodedKey,AESKeyLength/8);
            //fprintf(fkey,"%s\n",base64EncodedKey);
            base64EncodedCt=EncodedMACaesEnc(AESPt[i],AESKeys[i]);
            sprintf(str,"'%s',",base64EncodedCt);
	    strcat(sql,str);
	    //fprintf(fout,"%s",base64EncodedCt);
            //if(i!=l) fprintf(fout," ");
#ifdef VERBOSE
            printf("\tPlaintext %s\n",AESPt[i]);
            printf("\tAttr      %u\n",Attr[i]);
            element_printf("\tThe random element:\n\t%B\n",RowElements[i]);
            //printf("\tThe extended key:\n%s\n",extendedKey);
            printf("\tThe raw key:\n\t");
            for(int j=0;j<AESKeyLength/8;j++)printf("%02X ",AESKeys[i][j]);
            printf("\n");
            printf("\t%d The Pt:\n%s\t\n",i,AESPt[i]);
            printf("\t%d The encoded Ct:\n\t%s\n",i,base64EncodedCt);
#endif
        }
        sql[strlen(sql)-1]=0;
	sprintf(str,");");
	strcat(sql,str);
	rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
   	if( rc != SQLITE_OK ){
      		fprintf(stderr, "SQL error: %s\n ", zErrMsg);
      		sqlite3_free(zErrMsg);
		return rc;
   	}
        //fprintf(fout,"\n");
        Act_t *Ct=Aencrypt(pairing, SetUp->publicl,SetUp->public1,Attr,l,RowElements);
        storeAct(pairing,Ct,fenckey);
        return 0;
}

int
writeKRow(pairing_t *pairing, Asetup_t *SetUp, char *line, int l, FILE *fout, FILE *fkey, FILE *fenckey)
{


        element_t *RowElements=malloc((l+1)*sizeof(element_t));
        char **AESPt=malloc((l+1)*sizeof(char *));
        unsigned char **AESKeys=malloc((l+1)*sizeof(unsigned char **));
        int lentok, tokennum;
        char *token, *extendedKey, *base64EncodedKey, *base64EncodedCt;
        unsigned int *Attr;

        AESPt=parseLine(line,l);
        if (AESPt==NULL) return -1;
        char **hashAESPt=malloc((l+1)*sizeof(char *));
        for(int i=1;i<l+1;i++)
            hashAESPt[i]=sha256(AESPt[i]);
        Attr=malloc((2*l+2)*sizeof(int));
        for(int i=1;i<l+1;i++){
                Attr[i]=*((int *)hashAESPt[i]);
                Attr[l+1+i]=i;
        }

#ifdef VERBOSE
        for(int i=1;i<l+1;i++){
            printf("Plaintext %s\n",AESPt[i]);
            printf("Attr      %u\n",Attr[i]);
        }
#endif



        //for(tokennum=1, token=strtok(line," "); token; tokennum++, token=strtok(NULL," ")){
            //if (tokennum>l){
                    //fprintf(stderr,"Too many records (%d)\n",tokennum);
                    //return -1;
            //}
            //if (strlen(token)>MAXLengthByte){
                  //fprintf(stderr,"Record too long: %s\n",token);
                  //return -1;
            //}
            /* remove \n from last record of the row */
            //lentok=strlen(token);
            //if (token[lentok-1]=='\n') token[lentok-1]='\0';
            //AESPt[tokennum]=malloc((lentok+1)*sizeof(char));
            //strcpy(AESPt[tokennum],token);
        //}

        //tokennum--;
        //if (tokennum<l){
            //fprintf(stderr,"Too few records (%d)\n",tokennum);
            //return -1;
        //}

        /* now we have l Pt in AESPt to be encrypted */
        for (int i=1;i<l+1;i++){
            /* let us pick a key to encrypt AESPt[i] */
#ifdef VERBOSE
            printf("Column=%d\n",i);
#endif
            element_init_GT(RowElements[i],*pairing);
            element_random(RowElements[i]);
            extendedKey=malloc(MAXLengthByte);
            element_to_bytes(extendedKey,RowElements[i]);
            AESKeys[i]=malloc(AESKeyLength/8*sizeof(unsigned char));
            memcpy(AESKeys[i],extendedKey+1,AESKeyLength/8);
            Base64Encode((const unsigned char *)AESKeys[i], (char **) &base64EncodedKey,AESKeyLength/8);
            //fprintf(fkey,"%s\n",base64EncodedKey);
            base64EncodedCt=EncodedMACaesEnc(AESPt[i],AESKeys[i]);
            fprintf(fout,"%s",base64EncodedCt);
            if(i!=l) fprintf(fout," ");

#ifdef VERBOSE
            printf("\tPlaintext %s\n",AESPt[i]);
            printf("\tAttr      %u\n",Attr[i]);
            element_printf("\tThe random element:\n\t%B\n",RowElements[i]);
            //printf("\tThe extended key:\n%s\n",extendedKey);
            printf("\tThe raw key:\n\t");
            for(int j=0;j<AESKeyLength/8;j++)printf("%02X ",AESKeys[i][j]);
            printf("\n");
            printf("\t%d The Pt:\n%s\t\n",i,AESPt[i]);
            printf("\t%d The encoded Ct:\n\t%s\n",i,base64EncodedCt);
#endif
        }

        fprintf(fout,"\n");

        Act_t *Ct=Aencrypt(pairing, SetUp->publicl,SetUp->public1,Attr,l,RowElements);
        storeAct(pairing,Ct,fenckey);
        return 0;
}

int
writeRow(pairing_t *pairing, Asetup_t *SetUp,char *line, int l, FILE *fout, FILE *fkey, FILE *fenckey)
{


        element_t *RowElements=malloc((l+1)*sizeof(element_t));
        //char **AESPt=malloc((l+1)*sizeof(char *));
        char **AESPt=malloc((l+1)*sizeof(char *));
        unsigned char **AESKeys=malloc((l+1)*sizeof(unsigned char **));
        int lentok, tokennum;
        char *token, *extendedKey, *base64EncodedKey, *base64EncodedCt;
        int *Attr;

        Attr=malloc((2*l+2)*sizeof(int));
        for(int i=1;i<l+2;i++){Attr[i]=i;Attr[l+i+1]=i;}
        AESPt=parseLine(line,l);
        if (AESPt==NULL) return -1;

        //for(tokennum=1, token=strtok(line," "); token; tokennum++, token=strtok(NULL," ")){
            //if (tokennum>l){
                    //fprintf(stderr,"Too many records (%d)\n",tokennum);
                    //return -1;
            //}
            //if (strlen(token)>MAXLengthByte){
                  //fprintf(stderr,"Record too long: %s\n",token);
                  //return -1;
            //}
            /* remove \n from last record of the row */
            //lentok=strlen(token);
            //if (token[lentok-1]=='\n') token[lentok-1]='\0';
            //AESPt[tokennum]=malloc((lentok+1)*sizeof(char));
            //strcpy(AESPt[tokennum],token);
        //}

        //tokennum--;
        //if (tokennum<l){
            //fprintf(stderr,"Too few records (%d)\n",tokennum);
            //return -1;
        //}

        /* now we have l Pt in AESPt to be encrypted */
        for (int i=1;i<l+1;i++){
            /* let us pick a key to encrypt AESPt[i] */
            element_init_GT(RowElements[i],*pairing);
            element_random(RowElements[i]);
            extendedKey=malloc(MAXLengthByte);
            element_to_bytes(extendedKey,RowElements[i]);
            AESKeys[i]=malloc(AESKeyLength/8*sizeof(unsigned char));
            memcpy(AESKeys[i],extendedKey+1,AESKeyLength/8);
            Base64Encode((const unsigned char *)AESKeys[i], (char **) &base64EncodedKey,AESKeyLength/8);
            fprintf(fkey,"%s\n",base64EncodedKey);
            base64EncodedCt=EncodedMACaesEnc(AESPt[i],AESKeys[i]);
            fprintf(fout,"%s",base64EncodedCt);
            if(i!=l) fprintf(fout," ");
            element_printf("%d The random element:\n%B\n",i,RowElements[i]);
            //printf("%d The extended key:\n%s\n",i,extendedKey);
            printf("%d The raw key:\n",i);
            for(int j=0;j<AESKeyLength/8;j++)printf("%02X ",AESKeys[i][j]);
            printf("\n");
            printf("%d The Pt:\n%s\n",i,AESPt[i]);
            printf("%d The encoded Ct:\n%s\n",i,base64EncodedCt);
        }

        fprintf(fout,"\n");

        Act_t *Ct=Aencrypt(pairing, SetUp->publicl,SetUp->public1,Attr,l,RowElements);
        storeAct(pairing,Ct,fenckey);
        return 0;
}

unsigned char **
readRow(pairing_t *pairing, Asetup_t *SetUp,char *line, int l, FILE *fkey)
{

        unsigned char *ExtendedKey, *dec_out, *key;

        char **AESCt=parseLine(line,l);
        unsigned char **AESKey=malloc((l+1)*sizeof(unsigned char*));
        unsigned char **Pt=malloc((l+1)*sizeof(unsigned char*));
        Act_t *EncAESKeys=readAct(pairing,fkey);
        int *Y=malloc((l+2)*sizeof(int));
        for (int i=1;i<l+1;i++) Y[i]=i;
        for(int j=1;j<l+1;j++){
            Y[l+1]=j;
            Akey_t *Key=Akeygen(pairing,SetUp->privatel,SetUp->private1,"",Y);
            element_t *mOut=ASingdecrypt(pairing,EncAESKeys,Key,j);
            //element_printf("decrypted message: %B\n",*mOut);
            ExtendedKey=malloc(MAXLengthByte);
            element_to_bytes(ExtendedKey,*mOut);
            AESKey[j]=malloc(AESKeyLength/8);
            memcpy(AESKey[j],ExtendedKey+1,AESKeyLength/8);
            Pt[j]=EncodedMACaesDec(AESCt[j],AESKey[j]);
            //printf("Raw key for %d: ",j);
            //for(int kk=0;kk<AESKeyLength/8;kk++) printf("%02X ",(unsigned char)AESKey[j][kk]);
            //printf("\n");
            //printf("%d The decoded CT: %s\n",j,Pt[j]);
        }
        return Pt;
}
