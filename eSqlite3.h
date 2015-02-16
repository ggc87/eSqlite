#ifndef ESQLITE3_H
#define ESQLITE3_H

#include<sqlite3.h>

#include "hve_amm.h"


typedef sqlite3 eSqlite3;

//static Akey_t **_tokens

//char p;

void setP(char a);

//setup system
int eSqlite3_setUp(
	eSqlite3 *db, 	/*An open db*/
	char *pairingFile,	/*Pairing filename*/
	int l, 			/*columns number*/
	const char *filename 	/*keys filename */
	);

//create token for an SQL select query
int eSqlite3_selectGenToken(
	sqlite3* db,                                  /* An open database */
	char *pairingFile,	/*Pairing filename */
	const char *fmsk,	/* Master secret key filename */
	const char *tok,	/* Token filename */
	const char *sql		/* SQL select query */
	);

//exec a list of sql commands
int eSqlite3_exec(
  sqlite3* db,                                  /* An open database */
  const char *sql,                           /* SQL to be evaluated */
  int (*callback)(void*,int,char**,char**),  /* Callback function */
  void *,                                    /* 1st argument to callback */
  char **errmsg,                             /* Error msg written here */
  char *pairing,			     /* Pairing filename */
  char *mpk, 				     /* Masert public key filename */
  Akey_t **tokens,
  int size
);

//exec an INSERT command
int eSqlite3_encrypt(
	eSqlite3 *db,		/*An open database*/
	char *pairinig,		/*Pairing filename */
	char *mpk,		/*Master public key filename*/
	char *sql,		/*sql insert command*/
	char **errmsg          /* Error msg written here */
	);

//create a token for a SELECT command
int eSqlite3_addToken(
	char *pairingstr, /* pairing filename */
	char **tokensFile,    /*list of tokens filename*/
	Akey_t ***tokens,  /*list of open token*/
	int n,		  /*number of open token*/
	int size         /*size of tokens filename list*/
	);

//exec a SELECT command
int eSqlite3_select(
		eSqlite3 *db,
		char *sql,
	        char *pairingFile,
		Akey_t **tokens,
  		int (*callback)(void*,int,char**,char**),  /* Callback function */
		int size
		);


#define eSqlite3_open(filename,eDb) sqlite3_open(filename,eDb)
#define eSqlite3_close(eDb) sqlite3_close(eDb)



#endif
