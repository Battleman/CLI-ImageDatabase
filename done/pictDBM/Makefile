DEBUG=yes
CC=gcc
CFLAGS += -Wall -pedantic -g -std=c99 -lssl -lcrypto
CFLAGS += $$(pkg-config vips --cflags)
ifeq ($(DEBUG), yes)
	CFLAGS += -g
endif
MAIN_EXEC = pictDBM	
LDLIBS += $$(pkg-config vips --libs) -lm


all: $(MAIN_EXEC)

error.o: error.c
db_list.o: db_list.c pictDB.h error.h
db_create.o: db_create.c pictDBM_tools.h pictDB.h
db_utils.o: db_utils.c pictDB.h error.h
db_delete.o: db_delete.c pictDB.h error.h
pictDBM_tools.o : pictDBM.c pictDB.h pictDB.h error.h
pictDBM.o: pictDBM.c pictDB.h pictDBM_tools.h error.h 
image_content.o : image_content.c image_content.h pictDB.h error.h 
dedup.o : dedup.c dedup.h error.h pictDB.h

$(MAIN_EXEC): pictDBM.o image_content.o db_list.o db_utils.o error.o db_create.o db_delete.o dedup.o pictDBM_tools.o

# scripts to facilitate cleaning and purging#
#################################################
.PHONY: clean mrproper stylish

#remove object files
clean:
	@rm -rf *.o
	@echo "Suppression des fichiers objets (exemple.o)"

#3 last lines are here to "reset" the provided
#databases, and removed other ones. Essentially
#temporary. Made to be totally cleaned and 
#ready to be graded.
mrproper : clean
	@rm -rf $(MAIN_EXEC)
	@echo "Suppression des executables"
	@rm -rf done/pictDBM/html #doxygen
	@rm -rf done/pictDBM/latex #doxygen
	@echo "Suppression des fichiers liés à Doxygen"
	@rm *.pictdb_static
	@cp ../../provided/week04/testDB01.pictdb_static .
	@cp ../../provided/week04/testDB02.pictdb_static .
	@cp ../../provided/week07/testDB02.pictdb_dynamic .
#	@cp ../../provided/week09/coquelicots.jpg
#	@cp ../../provided/week09/foret.jpg
#	@cp ../../provided/week09/papillon.jpg
	@echo "Réinitialisation des images et bases de donnée"
	@rm -rf *~
	@echo "Suppression de fichiers de sauvegarde"
	@echo "--- \nMr Proper a rendu votre repo tout propre !"
#to apply astyle -A8 ; script is not provided, only 
#used before handing back.
stylish:
	./stylish.sh
