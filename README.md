# eSqlite
The main work has been made by University of Salerno
http://libeccio.di.unisa.it/SecureQuery/

____


#Secure Query
#Introduction
eSqlite is a cryptographic library that can be used to encrypt tables so that the owner of the table can release tokens to allow third parties to perform queries on the encrypted table. An eSqlite program try to have the same structure as an sqlite program, so users can perform SQL query on encrypted data. SQL query are used in the creation of tokens as well.
In the example folder it is possible to find some example program which represent the typical workflow:

    setUpPhase <dbname> <pairing> <l> <keyfilename>
        <dbname> name of the database to create.
        <pairing> is the name of the file containing a description of the bilinear setting used. It is possible to use any of the files in the Curves directory of the distribution (which in turn are taken from the PBC Library).
        <l> is the number of columns of each row.
        <keyfilename> the master public key will be written in file keyfilename.mpk and the master secret key will be written in file keyfilename.msk

    tokenGeneration <dbname> <pairing> <master secret key> <token>
        <dbname> name of the database to create.
        <pairing> is the name of the file containing a description of the bilinear setting used. It is possible to use any of the files in the Curves directory of the distribution (which in turn are taken from the PBC Library).
        <master secret key> is the name of the file containing the master secret key to be used in the encryption;
        <token> is the name of the file that will contain the token

    insert <dbname> <pairing> <masete public key>
        <dbname> name of the database to create.
        <pairing> is the name of the file containing a description of the bilinear setting used. It is possible to use any of the files in the Curves directory of the distribution (which in turn are taken from the PBC Library).
        <master public key> is the name of the file containing the master public key to be used in the encryption;

    select <dbname> <pairing>
        <dbname> name of the database to create.
        <pairing> is the name of the file containing a description of the bilinear setting used to generate the master public and secret keys used to encrypt and generate the tokens;

Consider the following simple table:

	"ID INT PRIMARY KEY     NOT NULL," 
         "NAME           TEXT    NOT NULL," 
         "AGE            INT     NOT NULL," 
         "ADDRESS        CHAR(50)," 
         "SALARY         REAL
   

    The command setupPhase db ../Curves/e.param 5 key generates master public key key.mpk and master secret key key.msk, create a new database db and execute a CREATE SQL query to create the table COMPANY;
    The command tokenGeneration db ../Curves/e.param key token generates a new token written into the file "token" which a user can use to execute the following SELECT query: SELECT NAME FROM COMPANY WHERE AGE=32;
    The command insert db ../Curves/e.param key will ask for a new entry for the table COMPANY. The new data will be encrypted and the encrypted data will be written into COMPANY.knc and COMPANY.hve; Suppose we insert the following data
        (1, 'Paul', 32, 'California', 20000 )
        (2, 'Allen', 25, 'Texas', 15000 )
        (3, 'Teddy', 23, 'Norway', 20000 )
        (4, 'Luke', 32, 'Rich-Mond ', 65000 );
    The command select db ../Curves/e.param will use the token "token" generated by tokenGeneration to perform the following SELECT query:SELECT NAME FROM COMPANY WHERE AGE=32. The output of this command will be:

    			NAME:Paul
    			NAME:Luke	
    		

#eSqlite API

        int eSqlite3_open(       /* Open databse */ 
          const char *filename,   /* Database filename (UTF-8) */
          eSqlite3 **ppDb          /* OUT: eSQLite db handle */
        );
        	
        							

        int eSqlite3_close(       /* Close databse */
          eSqlite3 *ppDb          /* An open db */
        );
        	
        							

        int eSqlite3_setUp(       /* Setup the system creating the master and secret key and a new databse */
           eSqlite3 *db,   	/*An open db*/
           char *pairingFile,      /*Pairing filename*/
           int l,                  /*columns number*/
           const char *filename    /*keys filename */
        );
        				

        int eSqlite3_selectGenToken(    /* Generate a new token from an SQL query using the master secret key */
                sqlite3* db,             /* An open database */
                char *pairingFile,      /*Pairing filename */
                const char *fmsk,       /* Master secret key filename */
                const char *tok,        /* Token filename */
                const char *sql         /* SQL select query */
        );

        				

        int eSqlite3_exec(          /* Exec an SQL query */
          sqlite3* db,                                  /* An open database */
          const char *sql,                           /* SQL to be evaluated */
          int (*callback)(void*,int,char**,char**),  /* Callback function */
          void *arg,                                    /* 1st argument to callback */
          char *pairing,                             /* Pairing filename */
          char *mpk,                                 /* Masert public key filename */
          Akey_t **tokens,				/* List of open token */
          int size				/* size of tokens */
        );
        				

        int eSqlite3_addToken(    /* Add a new token to the tokens array */ 
                char *pairingstr, /* pairing filename */
                char **tokensFile,    /*list of tokens filename*/
                Akey_t ***tokens,  /*list of open token*/
                int n,            /*number of open token*/
                int size         /*size of tokens filename list*/
        );

        				


