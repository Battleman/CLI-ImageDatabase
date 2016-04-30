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
int do_create(const char* filename, struct pictdb_file* db_file, uint32_t max_files, uint16_t thumb_res_X, uint16_t thumb_res_Y, 
uint16_t small_res_X, uint16_t small_res_Y)
{
    struct pictdb_header header = db_file -> header;
    db_file -> fpdb = fopen(filename, "wb");
    int errcode = 0;
    
    if(NULL == db_file -> fpdb){
		return ERR_IO;
	}
    
    //initialisation des valeurs par défaut
    strncpy	(header.db_name, CAT_TXT,  MAX_DB_NAME); //copie du nom par défaut
    (header).db_name[MAX_DB_NAME] = '\0'; //la "string" doit se terminer par \0
    
    header.db_version = 0;
    header.num_files = 0;
    header.db_max_files = max_files;
    
	header.res_resized[2 * RES_THUMB] = thumb_res_X;
	header.res_resized[2 * RES_THUMB + 1] = thumb_res_Y;
	header.res_resized[2 * RES_SMALL] = small_res_X;
	header.res_resized[2 * RES_SMALL + 1] = small_res_Y;
	
	//initialisation du fichier et de la mémoire
	if(1 == fwrite(&db_file->header, sizeof(struct pictdb_header), 1, db_file->fpdb)) {
		printf("%u item(s) written\n", sum);
	} else {
		errcode = ERR_IO; //couldn't write all the elements (or wrote too much) -> don't return 0 anymore (??!)
	}
	
	if (errcode == 0 && NULL == db_file -> metadata = calloc(max_files, sizeof(struct pict_metadata))){
		errcode = ERR_OUT_OF_MEMORY;
	}
	
	do_close(db_file);
	
    return errcode; // retourne l'erreur;
}
