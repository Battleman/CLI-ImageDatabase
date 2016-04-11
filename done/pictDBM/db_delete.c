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
	rewind(fpdb);
    fseek(fpdb, sizeof(struct pictdb_header), SEEK_SET);
    int valid = 0, index = 0;
    struct pict_metadata local;
    while(SEEK_CUR < SEEK_END && valid == 0) {
		fread(&local, sizeof(struct pict_metadata), 1, fpdb);
		if(strcmp(local.pict_id, meta_table[index].pict_id) == 0) {
			struct pict_metadata new = meta_table[index];
			new.is_valid = 0;
			fseek(fpdb, -sizeof(struct pict_metadata), SEEK_CUR);
			valid = fwrite(&new, sizeof(struct pict_metadata), 1, fpdb);
		}
		index++;
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
