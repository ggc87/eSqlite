#include <stdio.h>
#include "../eSqlite3.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      printf(" %s-%s\n", azColName[i],argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int main(int argc, char* argv[])
{
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char *sql;

  if(argc!=5){
  	fprintf(stderr,"Usage: tokenGeneration <db> <pairing> <master secret key> <token>\n");
	return 1;
  }
  //sql SELECT query
  sql =	"SELECT NAME FROM COMPANY WHERE AGE=32;";
  //open database
  rc = eSqlite3_open(argv[1], &db);

   if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return 0;
   }else{
      fprintf(stderr, "Opened database successfully\n");
   }
   //the token file argv[4] will allow users to decrypt
   // the column NAME if and only if AGE is equal to 32 
   //eSqlite3_selectGenToken need to know pairing filename argv[2]
   //master secret key filename argv[3] 
   //output token filename argv[4]
   eSqlite3_selectGenToken(db,argv[2],argv[3],argv[4],sql);
   //close db 
   eSqlite3_close(db);
}
