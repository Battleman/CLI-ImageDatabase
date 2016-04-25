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
int do_create(const char* filename, struct pictdb_file* db_file, uint32_t max_files, uint16_t[] thumb_res, uint16_t[] small_res)
{
    pictdb_header header = db_file -> header;
    
    //initialisation des valeurs par défaut
    strncpy	(header.db_name, CAT_TXT,  MAX_DB_NAME); //copie du nom par défaut
    (header).db_name[MAX_DB_NAME] = '\0'; //la "string" doit se terminer par \0
    
    header.db_version = header.num_files = 0;
    
	header.res_resized[0] = header.res_resized[1] = thumb_res;
	header.res_resized[2] = header.res_resized[3] = small_res;
	
	//initialisation du fichier et de la mémoire
	if(1 == fwrite(&db_file->header, sizeof(struct pictdb_header), 1, db_file->fpdb)) {
		printf("%u item(s) written\n", sum);
	} else {
		return ERR_IO; //couldn't write all the elements (or wrote too much) -> don't return 0 anymore (??!)
	}
	
	if (NULL == db_file -> metadata = calloc(max_files, sizeof(struct pict_metadata))){
		return  ERR_OUT_OF_MEMORY;
	}
	
    return 0; //no error;
}
