PBCLDFLAGS=-lpbc -lgmp -lsqlite3 

CFLAGS= -std=c99 -I/usr/local/ssl/include   -g -lsqlite3 
LDFLAGS= -L/usr/local/ssl/lib/ -L/usr/local/lib/ -lcrypto -ldl -lm 

OBJ= hve_amm.o setupA.o utils.o Base64Encode.o Base64Decode.o Encrypt.o sha256.o aesU.o Select.o GenToken.o 
EXE= Base64Encode.o Base64Decode.o sha256.o utils.o hve_amm.o eSqlite3.o lex.yy.o aesU.o 

all: ${EXE} 

Base64Encode.o: Base64Encode.c
	gcc -c ${CFLAGS} Base64Encode.c 

Base64Decode.o: Base64Decode.c
	gcc -c ${CFLAGS} Base64Decode.c 

sha256.o: sha256.c
	gcc -c ${CFLAGS} sha256.c 

utils.o: utils.c
	gcc -c ${CFLAGS} utils.c

hve_amm.o: hve_amm.c
	gcc -c ${CFLAGS} hve_amm.c

setupA.o: setupA.c 
	gcc -c ${CFLAGS} setupA.c

Encrypt.o: Encrypt.c
	gcc -c ${CFLAGS} Encrypt.c 

Select.o: Select.c
	gcc -c ${CFLAGS} Select.c 

eSqlite3.o: eSqlite3.c
	gcc -c ${CFLAGS} eSqlite3.c 

lex.yy.o: lex.yy.c
	gcc -c ${CFLAGS} lex.yy.c  

GenToken.o: GenToken.c
	gcc -c ${CFLAGS} GenToken.c 

setupA: setupA.o utils.o hve_amm.o
	gcc  setupA.o utils.o hve_amm.o -o $@ ${PBCLDFLAGS}

Encrypt: Encrypt.o Base64Encode.o Base64Decode.o utils.o hve_amm.o aesU.o sha256.o
	gcc -o $@ Encrypt.o Base64Encode.o Base64Decode.o utils.o hve_amm.o aesU.o sha256.o ${LDFLAGS} ${PBCLDFLAGS}
    
GenToken: GenToken.o Base64Encode.o Base64Decode.o utils.o hve_amm.o aesU.o sha256.o
	gcc -o $@ GenToken.o Base64Encode.o Base64Decode.o utils.o hve_amm.o aesU.o sha256.o ${LDFLAGS} ${PBCLDFLAGS}
    
Select: Select.o Base64Encode.o Base64Decode.o utils.o hve_amm.o aesU.o sha256.o
	gcc -o $@ Select.o Base64Encode.o Base64Decode.o utils.o hve_amm.o aesU.o sha256.o ${LDFLAGS} ${PBCLDFLAGS}
    


clean:  
	rm -f ${OBJ} 
	rm -f ${EXE}

