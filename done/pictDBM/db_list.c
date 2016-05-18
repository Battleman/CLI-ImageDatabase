/**
 * @file db_list.c
 * @brief pictDB library: do_list implementation.
 */
#include "pictDB.h"
/********************************************************************//**
 * Iterates through the database and prints the header and each
 * valid database entry
 */
const char* do_list(const struct pictdb_file* db_file, enum do_list_mode mode)
{	if(mode == STDOUT) {
		if(db_file != NULL) {
			int empty = 1; //check if there is an entry in the database
	
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
		return NULL;
	}
	if(mode == JSON) {
		const char name_str[] = "Picture";
		struct json_object* objet = json_object_new_object();
		struct json_object* name_obj = json_object_new_string(name_str);
		struct json_object* array = json_object_new_array();
		for(size_t i = 0; i < db_file->max_files, i++) {
			struct json_object* pict_name = json_object_bew_string(db_file->metadata[i].pict_id);
			json_object_array_add(array, pict_name);
		}
		const char* string = json_object_to_json_string (object);
		return string;
	}
	const char err_msg[] = "unimplemented do_list mode";
	return err_msg;
}
