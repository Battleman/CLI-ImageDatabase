#include "error.h"
#include "pictDB.h"
#include "image_content.h"

int do_name_and_content_dedup(struct pictdb_file* db_file, uint32_t index)
{
    if(db_file == NULL) {
        return ERR_IO;
    } else if(0 == (db_file -> metadata[index].is_valid)) {
        return 0;
    }

    for(int i = 0; i < db_file -> header.max_files; i++) {
        if((0 != db_file -> metadata[i].is_valid) && (i != index)) {
            if(0 == strcmp(db_file -> metadata[index].pict_id, db_file -> metadata[i].pict_id)) {
                return ERR_DUPLICATE_ID;
            } else if(0 == table_compare(db_file -> metadata[index].SHA, db_file -> metadata[i].SHA, SHA256_DIGEST_LENGTH)) {
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

    db_file -> metadata[index].offset[RES_ORIG] = 0;
    return 0;
}
