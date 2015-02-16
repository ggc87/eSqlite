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
   char sql[500];
   int id, age, salary;
   char name[100], address[100];

  if(argc!=4){
	fprintf(stderr,"Usage: %s <dbname> <pairing> <keyfilename>\n",argv[0]);
	return 1;
  }
  printf("Insert ID: "); scanf("%d",&id);
  printf("Insert AGE: "); scanf("%d",&age);
  printf("Insert NAME: "); scanf("%s",name);
  printf("Insert ADDRESS: "); scanf("%s",address);
  printf("Insert SALARY: "); scanf("%d",&salary);
  sprintf(sql,"INSERT INTO COMPANY (ID,AGE,NAME,ADDRESS,SALARY) VALUES (%d, %d,'%s','%s', %d );",id,age,name,address,salary); 

   //open database argv[1]
   rc = eSqlite3_open(argv[1], &db);

   if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return 0;
   }else{
      fprintf(stderr, "Opened database successfully\n");
   }
   //for INSERT query eSqlite3_exec need to know pairing filename argv[2]
   //and master secret key filename argv[3]
   rc = eSqlite3_exec(db,sql,callback,0,&zErrMsg,argv[2],argv[3],NULL,0);
   if( rc != SQLITE_OK )
	{
		printf("%s\n",zErrMsg);
	}
   //close db
   eSqlite3_close(db);
}
