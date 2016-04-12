/**
 * @file db_delete.c
 * @brief pictDB library: do_delete implementation.
 */
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
    do {
        if(strcmp(pic_name, meta_table[index].pict_id) == 0) { //comparaison entre le nom donné en argument et le nom de chaque metadata
            meta_table[index].is_valid = 0; //modification du bit de validité
            valid = 1; //indication qu'un match a été trouvé
        } else {
            index++;
        }
    } while(valid == 0 && index < MAX_MAX_FILES); //itération jusq'à la taille de meta_table ou jusqu'à trouver un match

    if(valid == 0) {
        return ERR_INVALID_PICID;
    } else {
        fseek(fpdb, sizeof(struct pictdb_header), SEEK_SET); //déplace la tête de lecture après le header
        fseek(fpdb, index*sizeof(struct pict_metadata), SEEK_CUR); //déplace la tête au niveau du metadata à lire (ième)
        valid = fwrite(&meta_table[index], sizeof(struct pict_metadata), 1, fpdb); //ecriture au niveau de la tête, avec test de validité
    }

    if(valid != 1) { //doit être 1, car écrit exactement un éléments juste avant
        return ERR_IO;
    }
    return 0;
}

int modify_header(FILE* fpdb, struct pictdb_header* header)
{
    header->num_files--; //réduction du nombre de fichiers
    header->db_version++; //version suivante
    rewind(fpdb); //retour au début du fichier pour écrire sur le header
    fwrite(header, sizeof(struct pictdb_header), 1, fpdb);

    return 0;
}
