/**
 * @file db_list.c
 * @brief pictDB library: do_list implementation.
 */
#include "pictDB.h"
/********************************************************************//**
 * Iterates through the database and prints the header and each
 * valid database entry
 */
void do_list(const struct pictdb_file* db_file)
{
	if(db_file != NULL) {
		int empty = 1; //check if there is an entry in the database

        //correcteur: vous pourriez optimiser en regardant tout de suite si num_file == 0
		print_header(&db_file->header); //anyway : print the header
		for(size_t i = 0; i < db_file -> header.max_files; ++i) { //go through the whole database
			if(db_file -> metadata[i].is_valid == NON_EMPTY) {
				print_metadata(&db_file -> metadata[i]);
				empty = 0;
			}
		}

		if(empty == 1) {
			printf("<<empty database>>\n");
		}
	}
}
