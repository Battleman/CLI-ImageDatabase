DEBUG=yes
CC=gcc
LIBS = vips openssl json-c
CFLAGS += -Wall -ansi -pedantic -std=c99 
CFLAGS += $$(pkg-config --cflags $(LIBS))
LDFLAGS += -L./libmongoose
LDLIBS += $$(pkg-config --libs $(LIBS)) -lmongoose -ljson-c

ifeq ($(DEBUG), yes)
	CFLAGS += -g
endif
MAIN_EXEC = pictDBM	
SERVER_EXEC = pictDB_server
EXEC_LIST = $(MAIN_EXEC) $(SERVER_EXEC)

all: $(MAIN_EXEC)
everything: $(EXEC_LIST)

#Common to both exec
db_list.o: db_list.c pictDB.h error.h
db_read.o : db_read.c image_content.h pictDB.h error.h
db_utils.o: db_utils.c pictDB.h error.h
db_delete.o: db_delete.c pictDB.h error.h
pictDBM_tools.o : pictDBM.c pictDB.h pictDB.h error.h
pictDBM.o: pictDBM.c pictDB.h pictDBM_tools.h image_content.h error.h
image_content.o : image_content.c pictDB.h image_content.h dedup.h error.h 
dedup.o : dedup.c dedup.h pictDB.h error.h
db_insert.o : db_insert.c pictDB.h image_content.h error.h 

COMMON_OBJ = db_list.o image_content.o db_utils.o error.o db_delete.o dedup.o pictDBM_tools.o dedup.o db_insert.o db_read.o

db_create.o: db_create.c pictDBM_tools.h pictDB.h error.h
$(MAIN_EXEC): $(COMMON_OBJ) pictDBM.o db_create.o


pictDB_server.o : pictDB_server.c pictDB.h error.h
$(SERVER_EXEC) : $(COMMON_OBJ) pictDB_server.o

# scripts to facilitate cleaning and purging#
#################################################
.PHONY: clean mrproper stylish

#remove object files
clean:
	@rm -rf *.o
	@echo "Suppression des fichiers objets (exemple.o)"

mrproper : clean
	@rm -rf $(EXEC_LIST)
	@echo "Suppression des executables"
	@rm -rf html/ #doxygen
	@rm -rf latex/ #doxygen
	@echo "Suppression des fichiers liés à Doxygen"
	@rm *.pictdb_*
	@cp ../../provided/week04/testDB01.pictdb_static .
	@cp ../../provided/week04/testDB02.pictdb_static .
	@cp ../../provided/week07/testDB02.pictdb_dynamic .
	@rm *.jpg
	@cp ../../provided/week09/coquelicots.jpg .
	@cp ../../provided/week09/foret.jpg .
	@cp ../../provided/week09/papillon.jpg .
	@echo "Réinitialisation des images et bases de donnée"
	@rm -rf *~
	@echo "Suppression de fichiers de sauvegarde"
	@echo "--- \nMr Proper a rendu votre repo tout propre !"
#to apply astyle -A8 ; script is not provided, only 
#used before handing back.
stylish:
	./stylish.sh

problem_search:
	./flaw.sh
