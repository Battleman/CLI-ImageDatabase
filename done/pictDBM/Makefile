CFLAGS += -std=c99 -lssl -lcrypto

all: pictDBM

error.o: error.c
db_list.o: db_list.c pictDB.h error.h
db_create.o: db_create.c pictDB.h
db_utils.o: db_utils.c pictDB.h error.h
db_delete.o: db_delete.c pictDB.h error.h
pictDBM.o: pictDBM.c pictDB.h error.h
 

pictDBM: pictDBM.o db_list.o db_utils.o error.o db_create.o db_delete.o
	gcc -g -o pictDBM pictDBM.o db_list.o db_utils.o error.o db_create.o db_delete.o
		
	

