#include "pictDB.h"

void do_list(struct pictdb_file myfile) {
	int empty = 1; //check if there is an entry in the database
	
	print_header(myfile.header); //anyway : print the header
    // Correcteur : privilégier uint32_t ou size_t
    // Correcteur : Pas une bonne pratique d'utiliser une constante comme taille de tableau
    // car en cas de refactorting, il faudra vérifier TOUS les endroits utilisant cette 
    // constante quand on change la tailles d'un tableau. Que se passe-t-il si on veut 
    // utiliser des pictdb_file avec des metadata de taille différentes ? Utiliser
    // pictdb_file.header.max_files permet d'avoir la garantie d'avoir la bonne taille de
    // tableau sans complexifier un futur refactoring. 0.5/1pt
	for(int i = 0; i < MAX_MAX_FILES; ++i) { //go through the 
        // Correcteur : utiliser les constantes EMPTY/NON_EMPTY -> 1/2pts
		if(myfile.metadata[i].is_valid != 0) {
			print_metadata(myfile.metadata[i]);
			empty = 0;
		}
	}
	
	if(empty == 1) {
		printf("<<empty database>>\n");
	}
}
