#CCFLAGS=  -L /home/ggc/Dropbox/project/unisa/SecureQuery/svn/securequery/trunk -ldl -lsqlite3 -std=c99
PBCLDFLAGS=-lpbc -lgmp -lsqlite3 

CFLAGS= -std=c99 -I/usr/local/ssl/include   -g -lsqlite3 
LDFLAGS= -L/usr/local/ssl/lib/ -L/usr/local/lib/ -lcrypto -ldl -lm 


CC=gcc
DEP=../utils.o ../hve_amm.o ../eSqlite3.o ../lex.yy.o ../aesU.o ../sha256.o  ../Base64Encode.o ../Base64Decode.o
EXE= setUpPhase insert select tokenGeneration

all: ${EXE}

dependencie: 
	cd ../; make


setUpPhase: setUpPhase.o dependencie
	  ${CC} setUpPhase.o ${DEP}  -lfl -o $@ ${LDFLAGS} ${PBCLDFLAGS}

tokenGeneration: dependencie tokenGeneration.o
	  ${CC} tokenGeneration.o ${DEP}  -lfl -o $@ ${LDFLAGS} ${PBCLDFLAGS}

insert:	dependencie insert.o
	  ${CC} insert.o ${DEP}  -lfl -o $@ ${LDFLAGS} ${PBCLDFLAGS}

select:	dependencie select.o
	  ${CC} select.o ${DEP}  -lfl -o $@ ${LDFLAGS} ${PBCLDFLAGS}

clean:
	rm -f ${EXE}
	rm -f *.o
