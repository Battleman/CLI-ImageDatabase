#include "error.h"
#include "pictDB.h"

int modify_reference(const char* filename, FILE* fpdb, struct pict_metadata* meta_table);
int modify_header(FILE* fpdb, struct pictdb_header* header);

int do_delete(const char* pic_name, struct pictdb_file* file)
{
    int err = modify_reference(pic_name, file -> fpdb, file -> metadata);
    if(err == 0) {
        err = modify_header(file -> fpdb, &file -> header);
    }
    return err;
}

int modify_reference(const char* pic_name, FILE* fpdb, struct pict_metadata* meta_table)
{
	int index, valid = 0;
	struct pict_metadata* new;
	for(int i = 0; i < MAX_MAX_FILES; i++){
		if(strcmp(pic_name, meta_table[i].pict_id) == 0){
			meta_table[i].is_valid = 0;
			new = &meta_table[i];
			index = i;
			valid = 1;
		}
	}
	
	if(valid != 0){
		fseek(fpdb, sizeof(struct pictdb_header), SEEK_SET);
		fseek(fpdb, index*sizeof(struct pict_metadata), SEEK_CUR);
		valid = fwrite(new, sizeof(struct pict_metadata), 1, fpdb);
	}
	
	if(valid != 0){
		return 0;
	}

    return ERR_INVALID_PICID;
	
}

int modify_header(FILE* fpdb, struct pictdb_header* header)
{
	struct pictdb_header h = *header;
	h.num_files--;
	h.db_version++;
    rewind(fpdb);
    fwrite(&h, sizeof(struct pictdb_header), 1, fpdb);
    
    return 0;
}
