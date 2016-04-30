/**
 * @file db_list.c
 * @brief pictDB library: do_list implementation.
 */
#include "pictDB.h"
/********************************************************************//**
 * Iterates through the database and prints the header and each
 * valid database entry
 */
void do_list(const struct pictdb_file* myfile)
{
    int empty = 1; //check if there is an entry in the database
	
    print_header(&myfile->header); //anyway : print the header
    for(size_t i = 0; i < myfile -> header.max_files; ++i) { //go through the whole database
        if(myfile -> metadata[i].is_valid == NON_EMPTY) {
            print_metadata(&myfile -> metadata[i]);
            empty = 0;
        }
    }

    if(empty == 1) {
        printf("<<empty database>>\n");
    }
}
