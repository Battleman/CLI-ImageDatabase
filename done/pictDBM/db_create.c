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

	
	if(1 == fwrite(&db_file->header, sizeof(struct pictdb_header), 1, db_file->fpdb)) { //no metadata + 1 header
		printf("%u item(s) written\n", sum);
	} else {
		return ERR_IO; //couldn't write all the elements (or wrote too much) -> don't return 0 anymore (??!)
	}
	
	int max_files = (MAX_MAX_FILES > db_file -> header.max_files) ? db_file -> header.max_files : MAX_MAX_FILES;
	
	if (NULL == db_file -> metadata = calloc(max_files, sizeof(struct pict_metadata))){
		return  ERR_OUT_OF_MEMORY;
	}
	
    return 0; //no error;
}
