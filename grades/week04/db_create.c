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

	for(int i = 0; i < db_file.header.max_files; i++){
		db_file.metadata[i].is_valid = EMPTY;
	}

    FILE* db = fopen(filename, "wb");
    if(db == NULL){
		return ERR_FILE_NOT_FOUND;
	} else {
		int sum = 0;
		sum += fwrite(&db_file.header, sizeof(struct pictdb_header), 1, db);
		sum += fwrite(&db_file.metadata, sizeof(struct pict_metadata), MAX_MAX_FILES, db);
		
		if(sum == (MAX_MAX_FILES + 1)){
			printf("%d item(s) written\n", sum);
			return 0;
		} else {
			return ERR_MAX_FILES;
		}
	}
}
