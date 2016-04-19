CFLAGS += -std=c99 -lssl -lcrypto 
CFLAGS += $$(pkg-config vips --cflags)
LDLIBS += $$(pkg-config vips --libs) -lm
#CFLAGS += -I/usr/include/glib-2.0/
#CFLAGS += -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/

all: pictDBM

error.o: error.c
db_list.o: db_list.c pictDB.h error.h
db_create.o: db_create.c pictDB.h
db_utils.o: db_utils.c pictDB.h error.h
db_delete.o: db_delete.c pictDB.h error.h
pictDBM.o: pictDBM.c pictDB.h error.h 
image_content.o : image_content.c image_content.h pictDB.h error.h 

pictDBM: pictDBM.o db_list.o db_utils.o error.o db_create.o db_delete.o image_content.o
	gcc -g -o $@ $^ $(CFLAGS)	
	
#afew scripts to facilitate cleaning and purging#
#################################################
.PHONY: clean mrproper stylish

#remove object files
clean:
	rm -rf *.o

#3 last lines are here to "reset" the provided
#databases, and removed other ones. Essentially
#temporary. Made to be totally cleaned and 
#ready to be graded.
mrproper : clean
	rm -rf $(EXEC)
	rm *.pictdb_static
	rm -rf done/pictDBM/html #doxygen
	rm -rf done/pictDBM/latex #doxygen
	cp ../../provided/week04/testDB01.pictdb_static .
	cp ../../provided/week04/testDB02.pictdb_static .

#to apply astyle -A8 ; script is not provided, only 
#used before handing back.
stylish:
	./stylish.sh
