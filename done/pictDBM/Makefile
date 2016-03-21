CFLAGS += -std=c99

all: pictDBM

error.o: error.c
db_list.o: db_list.c pictDB.h error.h
db_utils.o: db_utils.c pictDB.h error.h
pictDBM.o: pictDBM.c pictDB.h error.h

pictDBM: pictDBM.o db_list.o db_utils.o error.o
	gcc -o pictDBM pictDBM.o db_list.o db_utils.o error.o
