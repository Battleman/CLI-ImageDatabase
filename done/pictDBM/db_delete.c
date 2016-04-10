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
    /*for(int i = 0; i < MAX_MAX_FILES; i++) {
        if(meta_table[i].pict_id == filename) {
            //#################MIEUX IMPLÉMENTER########################
            fseek(fpdb, meta_table[i].offset[2], SEEK_SET);
            return 0;
        }
    }*/
    fseek(fpdb, sizeof(struct pictdb_header), SEEK_SET);
    int lecture_head = SEEK_CUR;
    int valid = 0;
    struct pict_metadata local;
    struct pict_metadata empty = {};
    while(lecture_head < SEEK_END && valid == 0) {
		fread(&local, sizeof(struct pict_metadata), 1, fpdb);
		if(strcmp(local.pict_id, meta_table->pict_id) == 0) {
			fwrite(&empty, sizeof(struct pict_metadata), 1, fpdb);
		}
	}

    return ERR_FILE_NOT_FOUND;
}

int modify_header(FILE* fpdb, struct pictdb_header* header)
{
    //Use fseek & fwrite
    return 0;
}
