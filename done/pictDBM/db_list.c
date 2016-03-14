#include "pictDB.h"

void do_list(struct pictdb_file myfile) {
	int bool = 0;
	
	print_header(myfile.header);
	for(int i = 0; i < MAX_MAX_FILES; ++i) {
		if(myfile.metadata[i].is_valid != 0) {
			print_metadata(myfile.metadata[i]);
			bool = 1;
		}
	}
	
	if(bool == 0) {
		printf("<<empty database>>");
	}
}
