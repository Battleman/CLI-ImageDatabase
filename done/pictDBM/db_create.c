/**
 * @file db_create.c
 * @brief pictDB library: do_create implementation.
 *
 * @author Mia Primorac
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <string.h> // for strncpy

/********************************************************************//**
 * Creates the database called db_filename. Writes the header and the
 * preallocated empty metadata array to database file.
 */
int do_create(const char* filename, struct pictdb_file* db_file)
{
    //initialisation des valeurs par défaut
    strncpy	(db_file -> header.db_name, CAT_TXT,  MAX_DB_NAME); //copie du nom par défaut
    (db_file -> header).db_name[MAX_DB_NAME] = '\0'; //la "string" doit se terminer par \0


	unsigned int sum = 0;
	sum += fwrite(&db_file->header, sizeof(struct pictdb_header), 1, db_file->fpdb);
	db_file->metadata = calloc(0, sizeof(struct pict_metadata)); //pas de metadata à la création de la db
	if(sum == 1) { //no metadata + 1 header
		printf("%u item(s) written\n", sum);
	} else {
		return ERR_MAX_FILES; //couldn't write all the elements (or wrote too much) -> don't return 0 anymore
	}
    return 0; //no error;
}
