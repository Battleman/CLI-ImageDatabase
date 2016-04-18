/**
 * @file db_create.c
 * @brief pictDB library: do_create implementation.
 *
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <string.h> // for strncpy

/********************************************************************//**
 * Creates the database called db_filename. Writes the header and the
 * preallocated empty metadata array to database file.
 */
int do_create(const char* filename, struct pictdb_file db_file){
    // Sets the DB header name
    strncpy(db_file.header.db_name, CAT_TXT,  MAX_DB_NAME);
    db_file.header.db_name[MAX_DB_NAME] = '\0';

	db_file.header.db_version = 0;
	db_file.header.num_files = 0;

    /* Correcteur : pas de vérification que header.max_files est valide
     * (cad. max_files <= MAX_MAX_FILES) -0.5pt
     */

	for(int i = 0; i < db_file.header.max_files; i++){
		db_file.metadata[i].is_valid = EMPTY;
	}

    /* Correcteur : db_file.fpdb déjà disponible pour ça. -0.25pts */
    FILE* db = fopen(filename, "wb");
    if(db == NULL){
        /* Correcteur : ce serait très étonnant que le fichier ne soit pas 
         * trouvé... car on crée un *nouveau* fichier. Ce serait plus une
         * erreur d'E/S qui se produirait. -0.25pt
         */
		return ERR_FILE_NOT_FOUND;
	} else {
		int sum = 0;
		sum += fwrite(&db_file.header, sizeof(struct pictdb_header), 1, db);
        /* Correcteur : metadata est déjà un pointeur en soit. Là c'est 
         * un pointeur de pointeur qui est envoyé. (et aussi,
         * utilisation de MAX_MAX_FILES au lieu header.max_files.) -1.5pt
         */
		sum += fwrite(&db_file.metadata, sizeof(struct pict_metadata), MAX_MAX_FILES, db);
		
        /* Correcteur : on devrait afficher le nombre réél d'objets écrits
         * (même en cas d'erreur) -0.5pt
         */
		if(sum == (MAX_MAX_FILES + 1)){
			printf("%d item(s) written\n", sum);
			return 0;
		} else {
            /* Correcteur : Ici on aurait plutôt une erreur d'E/S. -0.25pt */
			return ERR_MAX_FILES;
		}
	}
}
