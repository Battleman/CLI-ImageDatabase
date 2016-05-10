#include "error.h"
#include "pictDB.h"
#include "image_content.h"

int sha_compare(unsigned char orig_sha[], unsigned char comp_sha[])
{
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        if(orig_sha[i] != comp_sha[i]) {
            return 1;
        }
    }
    return 0;
}

int do_name_and_content_dedup(struct pictdb_file* db_file, uint32_t index)
{
    /* Correcteur : ne devrait jamais arriver */
    if(db_file == NULL) {
        return ERR_IO;
    } else if(0 == (db_file -> metadata[index].is_valid)) {
        return 0;
    }

    for(int i = 0; i < db_file -> header.max_files; i++) {
        /* Correcteur : utiliser les constantes EMPTY/NON EMPTY
         * -0.5pt
         */
        if((0 != db_file -> metadata[i].is_valid) && (i != index)) {
            if(0 == strcmp(db_file -> metadata[index].pict_id, db_file -> metadata[i].pict_id)) {
                return ERR_DUPLICATE_ID;
            } else if(0 == sha_compare(db_file -> metadata[index].SHA, db_file -> metadata[i].SHA)) {
                db_file -> metadata[index].offset[RES_ORIG] = db_file -> metadata[i].offset[RES_ORIG];
                db_file -> metadata[index].offset[RES_THUMB] = db_file -> metadata[i].offset[RES_THUMB];
                db_file -> metadata[index].offset[RES_SMALL] = db_file -> metadata[i].offset[RES_SMALL];
                db_file -> metadata[index].size[RES_THUMB] = db_file -> metadata[i].size[RES_THUMB];
                db_file -> metadata[index].size[RES_SMALL] = db_file -> metadata[i].size[RES_SMALL];
                db_file -> metadata[i].is_valid = 0;
                return 0;
            }
        }
    }

    /* Correcteur : à faire si et *seulement* si aucun doublon n'a été
     * trouvé
     * -0.5pt
     */
    db_file -> metadata[index].offset[RES_ORIG] = 0;
    return 0;
}
