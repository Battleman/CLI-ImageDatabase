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
    int index = 0, valid = 0;
    struct pict_metadata* new;
    do {
        if(strcmp(pic_name, meta_table[index].pict_id) == 0) { //comparaison entre
            meta_table[index].is_valid = 0;
            new = &meta_table[index];
            valid = 1;
        } else {
            index++;
        }
    } while(valid == 0 && index < MAX_MAX_FILES);

    if(valid == 0) {
        return ERR_INVALID_PICID;
    } else {
        fseek(fpdb, sizeof(struct pictdb_header), SEEK_SET); //déplace la tête de lecture après le header
        fseek(fpdb, index*sizeof(struct pict_metadata), SEEK_CUR); //déplace la tête au niveau du metadata à lire (ième)
        valid = fwrite(new, sizeof(struct pict_metadata), 1, fpdb); //ecriture au niveau de la tête, avec test de validité
    }

    if(valid != 1) { //doit être 1, car écrit un éléments juste avant
        return ERR_IO;
    }

    return 0;
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
