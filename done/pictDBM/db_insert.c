#include "pictDB.h"
#include <openssl/sha.h> // for SHA256_DIGEST_LENGTH

int do_insert(const char pict_id[], char* img, size_t size, struct pictdb_file* db_file){
	if(db_file == NULL || db_file->metadata == NULL) return ERR_INVALID_ARGUMENT;
	if(db_file->header.num_files >= db_file->header.max_files) return ERR_FULL_DATABASE;
	int found_empty = 0;
	int index = 0;
	while(index < db_file->header.max_files && !found_empty) {
		if(!db_file->metadata[index].is_valid) {
			found_empty = 1;
		} else {
			index++;
		}
	}
	
    (void)SHA256((unsigned char *)img, size, db_file->metadata[index].SHA);
    file->metadata[index].size[RES_ORIG] = (uint32_t)size;
	int errcode = 0;
	if(0 != (errcode = do_name_and_content(db_file, file->metadat[index].size[RES_ORIG]))) return errcode
	if(file->metadata[index].offset[RES_ORIG] == 0) {
		
	}
	return 0;
}
