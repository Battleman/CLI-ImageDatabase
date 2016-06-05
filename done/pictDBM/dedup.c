/**
 * @file dedup.c
 * @brief Recherche et suppression de doublons
 */

#include "error.h"
#include "pictDB.h"
#include "image_content.h"
/********************************************************************//**
 * Localisation de doublon
 */
int do_name_and_content_dedup(struct pictdb_file* db_file, const uint32_t index, const enum dedup_mode gbcollect)
{
    //vérification des inputs et des cas limites
    if(db_file == NULL || index < 0 || index >= db_file->header.max_files) return ERR_IO;

    //recherche de doublons
    int doublon = 0, errcode = 0;
    uint32_t from_meta_index = 0;
    uint32_t to_meta_index = 0;
    size_t valid_files = 0;
    for(uint32_t i = 0; i < db_file -> header.max_files; i++) {
        if((i != index) && (NON_EMPTY == db_file -> metadata[i].is_valid)) {
            valid_files++;
            //erreur dans le cas où 2 images portent le même nom
            if(!strncmp(db_file -> metadata[index].pict_id, db_file -> metadata[i].pict_id, MAX_PIC_ID)) {
                db_file->metadata[index].is_valid = EMPTY; //duplicata -> image invalide
                return ERR_DUPLICATE_ID;
            }
            //traitement des données dans le cas où un doublon a été trouvé
            if(0 == table_compare(db_file -> metadata[index].SHA, db_file -> metadata[i].SHA, SHA256_DIGEST_LENGTH)) {
                for(int res = 0; res < NB_RES; res++) {
                    //de base, on cherche une originale à copier dans le doublon
                    to_meta_index = index;
                    from_meta_index = i;
                    //si on a déjà une donnée dans le doublon, on la copie dans l'originale
                    if(db_file->metadata[to_meta_index].offset[res] != 0) {
                        to_meta_index = i;
                        from_meta_index = index;
                    }
                    db_file -> metadata[to_meta_index].offset[res] = db_file -> metadata[from_meta_index].offset[res];
                    db_file -> metadata[to_meta_index].size[res] = db_file -> metadata[from_meta_index].size[res];
                }
                db_file -> metadata[index].res_orig[0] = db_file -> metadata[i].res_orig[0];
                db_file -> metadata[index].res_orig[1] = db_file -> metadata[i].res_orig[1];
                doublon = 1;
            }
            if(gbcollect == GCOLLECT_ON) {
                if(	0 != (errcode = overwrite_metadata(db_file, to_meta_index)) ||
                    0 != (errcode = overwrite_metadata(db_file, from_meta_index))) return errcode;
            }
        }
    }


    //traitement du cas où aucun doublon n'a été trouvé
    if(doublon == 0 && gbcollect == GCOLLECT_OFF) {
        db_file -> metadata[index].offset[RES_ORIG] = 0;
    }

    return 0;
}
