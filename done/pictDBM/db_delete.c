/**
 * @file db_delete.c
 * @brief pictDB library: do_delete implementation.
 */
#include "error.h"
#include "pictDB.h"

/**
 * @brief suppression de l'image dans la metatable et réécriture de la metadata modifiée
 *
 * Itère dans la table pour trouver une image correspondant au nom passé en argument.
 * Pose le bit de validité à EMPTY puis, si l'image est trouvée, réécriture de la
 * metadata supprimée.
 *
 * @param pic_name Nom de l'image à supprimer
 * @param fpdb Le fichier dans lequel écrire
 * @param meta_table La metatable dans laquelle chercher l'image
 * @param size Taille metatable
 *
 * @return 0 en cas de succès, un code d'erreur sinon
 */
static int modify_reference(const char* pic_name, FILE* fpdb, struct pict_metadata* meta_table, size_t size)
{
    size_t index = 0;
    int valid = 0;
    do {
        if(meta_table[index].is_valid == NON_EMPTY && strcmp(pic_name, meta_table[index].pict_id) == 0) { //comparaison entre le nom donné en argument et le nom de chaque metadata
            meta_table[index].is_valid = EMPTY; //modification du bit de validité
            valid = 1; //indication qu'un match a été trouvé
        } else {
            index++;
        }
    } while(valid == 0 && index < size); //itération jusq'à la taille de meta_table ou jusqu'à trouver un match

    if(valid == 0) {
        return ERR_FILE_NOT_FOUND;
    } else {
        fseek(fpdb, sizeof(struct pictdb_header) + index * sizeof(struct pict_metadata), SEEK_SET); //déplace la tête de lecture
        valid = fwrite(&meta_table[index], sizeof(struct pict_metadata), 1, fpdb); //ecriture au niveau de la tête, avec test de validité
    }

    if(valid != 1) { //doit être 1, car écrit exactement un éléments juste avant
        return ERR_IO;
    }
    return 0;
}
/**
 * @brief modification du header de la DB
 *
 * Modifie le nombre de fichiers et la version (avec réécriture dans le fichier)
 *
 * @param fpdb Le fichier (ouvert) dans lequel écrire
 * @param header Le header à modifier puis à réécrire
 *
 * @return 0 en cas de succès, un code d'erreur sinon
 */
static int modify_header(FILE* fpdb, struct pictdb_header* header)
{
    if(header == NULL) return ERR_INVALID_ARGUMENT; //robustesse
    header->num_files--; //réduction du nombre de fichiers
    header->db_version++; //version suivante
    rewind(fpdb); //retour au début du fichier pour écrire sur le header

    if(1 != fwrite(header, sizeof(struct pictdb_header), 1, fpdb)) { //réécriture
        return ERR_IO;
    }

    return 0;
}
/********************************************************************//**
 * Supprime une image
 */
int do_delete(const char* pic_name, struct pictdb_file* file)
{
    int err = 0;
    if(file->header.num_files > 0) { //éviter des calculs si la base est vide
        err = modify_reference(pic_name, file -> fpdb, file -> metadata, file -> header.max_files);
        if(err == 0) {
            err = modify_header(file -> fpdb, &file -> header);
        }
    }
    return err;
}

