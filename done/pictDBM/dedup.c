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
int do_name_and_content_dedup(struct pictdb_file* db_file, uint32_t index)
{
    //vérification des inputs et des cas limites
    if(db_file == NULL) return ERR_IO;
    if(EMPTY == (db_file -> metadata[index].is_valid)) return 0;

    //recherche de doublons
    int doublon = 0;
    for(int i = 0; i < db_file -> header.max_files; i++) {
        if((i != index) && (NON_EMPTY == db_file -> metadata[i].is_valid)) {
            //erreur dans le cas où 2 images portent le même nom
            if(0 == strcmp(db_file -> metadata[index].pict_id, db_file -> metadata[i].pict_id)) {
                return ERR_DUPLICATE_ID;
            }

            //traitement des données dans le cas où un doublon a été trouvé
            if(0 == table_compare(db_file -> metadata[index].SHA, db_file -> metadata[i].SHA, SHA256_DIGEST_LENGTH)) {
                db_file -> metadata[index].offset[RES_ORIG] = db_file -> metadata[i].offset[RES_ORIG];
                db_file -> metadata[index].offset[RES_THUMB] = db_file -> metadata[i].offset[RES_THUMB];
                db_file -> metadata[index].offset[RES_SMALL] = db_file -> metadata[i].offset[RES_SMALL];
                db_file -> metadata[index].size[RES_THUMB] = db_file -> metadata[i].size[RES_THUMB];
                db_file -> metadata[index].size[RES_SMALL] = db_file -> metadata[i].size[RES_SMALL];
                db_file -> metadata[index].res_orig[0] = db_file -> metadata[i].res_orig[0];
                db_file -> metadata[index].res_orig[1] = db_file -> metadata[i].res_orig[1];
                db_file -> metadata[i].is_valid = EMPTY;
                doublon = 1;
            }
        }
    }

    //traitement du cas où aucun doublon n'a été trouvé
    if(doublon == 0) {
        db_file -> metadata[index].offset[RES_ORIG] = 0;
    }

    return 0;
}
