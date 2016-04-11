#include "error.h"
#include "pictDB.h"

int modify_reference(const char* filename, FILE* fpdb, struct pict_metadata* meta_table);
int modify_header(FILE* fpdb, struct pictdb_header* header);

int do_delete(const char* filename, struct pictdb_file* file)
{
    int valid = modify_reference(filename, file -> fpdb, file -> metadata);
    if(valid == 0) {
        valid = modify_header(file -> fpdb, &file -> header);
    }

    return valid;
}

int modify_reference(const char* filename, FILE* fpdb, struct pict_metadata* meta_table)
{
	int index, valid = 0;
	struct pict_metadata* new;
	for(int i = 0; i < MAX_MAX_FILES; i++){
		if(filename == meta_table[i].pict_id){
			meta_table[i].is_valid = 0;
			new = &meta_table[i];
			index = i;
			valid = 1;
		}
	}
	
	if(valid != 0){
		rewind(fpdb);
		fseek(fpdb, sizeof(struct pictdb_header), SEEK_SET);
		fseek(fpdb, index * sizeof(struct pictdb_header), SEEK_CUR);
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
