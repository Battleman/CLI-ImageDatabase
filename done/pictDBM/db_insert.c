/**
 * @file db_insert.c
 * @brief pictDB library: do_insert implementation.
 */

#include "pictDB.h"
#include "dedup.h"
#include "image_content.h"
#include <openssl/sha.h> // for SHA256_DIGEST_LENGTH

/********************************************************************//**
 * Insert an image
 */
int do_insert(const char pict_id[], char* img, size_t size, struct pictdb_file* db_file)
{
    //Vérification des input
    if(db_file == NULL || db_file->metadata == NULL || pict_id == NULL || !strcmp(pict_id, "")) return ERR_INVALID_ARGUMENT;
    if(db_file->header.num_files >= db_file->header.max_files) return ERR_FULL_DATABASE;
    int found_empty = 0, errcode = 0;
    size_t index = 0;

    //recherche d'un emplacement libre
    while(index < db_file->header.max_files && !found_empty) {
        if(!db_file->metadata[index].is_valid) {
            found_empty = 1;
        } else {
            index++;
        }
    }

    (void)SHA256(	(unsigned char*)img, 											//placement du SHA
                    size,
                    db_file->metadata[index].SHA
                );
    strncpy(db_file->metadata[index].pict_id, pict_id, MAX_PIC_ID); 				//copie de la pict_id
    db_file->metadata[index].size[RES_ORIG] = (uint32_t)size; 						//copie de la taille originale
    if(0 != (errcode = do_name_and_content_dedup(db_file, index))) return errcode; 	//recherche de doublon

    //si on a pas trouvé de doublon
    if(db_file->metadata[index].offset[RES_ORIG] == 0) {

        //déplacement à la fin pour écrire l'image et garder le offset
        if(0 != (errcode = fseek(db_file->fpdb, 0, SEEK_END))) return errcode;
        db_file->metadata[index].offset[RES_ORIG] = ftell(db_file->fpdb);
        if(1 != fwrite(img, size, 1, db_file->fpdb)) return ERR_IO;

        if(0 != (errcode = get_resolution(	&db_file->metadata[index].res_orig[1],
                                            &db_file->metadata[index].res_orig[0],
                                            (const char*) img,
                                            size
                                         )
                )
          )
            return ERR_RESOLUTIONS;
    }

    //une fois que le job est fini, l'image est confirmée valide
    db_file->metadata[index].is_valid = NON_EMPTY;
    //Qu'on trouve un doublon ou non, on màj la DB
    db_file->header.db_version++;
    db_file->header.num_files++;

    errcode = overwrite_metadata(db_file, index);
    if(errcode == 0) {
        errcode = overwrite_header(db_file->fpdb, &db_file->header);
    }


    return errcode;
}
