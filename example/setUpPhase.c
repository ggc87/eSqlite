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
	fprintf(stderr,"Usage: <dbname> <pairing> <l> <keyfilename>\n");
	return -1; 
   }

  sql = "CREATE TABLE COMPANY( ID INT PRIMARY KEY NOT NULL, NAME TEXT NOT NULL,          AGE            INT     NOT NULL,  ADDRESS        CHAR(50),      SALARY         REAL );"; 
   //open database argv[1]
   rc = eSqlite3_open(argv[1], &db);

   if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return 0;
   }else{
      fprintf(stderr, "Opened database successfully\n");
   }
   //system set up using pairing argv[2] for a table of argv[3] cols 
   //it will create a master secret key and a master publi key 
   //argv[4].msk argv[4].mpk
   eSqlite3_setUp(db,argv[2],atoi(argv[3]),argv[4]);
   //exec CREATE query
   //for CREATE query eSqlite3_exec need to know pairing filename
   //and master secret key filename
   rc = eSqlite3_exec(db,sql,callback,0,&zErrMsg,argv[2],argv[4],NULL,0);
   if( rc != SQLITE_OK )
	{
		printf("%s\n",zErrMsg);
	}
   //colse database
   eSqlite3_close(db);
}
