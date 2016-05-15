/**
 * @file db_read.c
 * @brief implementation of do_read
 */

#include "pictDB.h"
#include "image_content.h"

/********************************************************************//**
 * Lecture d'une image dans la résolution souhaitée et retour dans un buffer
 */
int do_read(const char pict_id[], const int RES, char** image_buffer, uint32_t* image_size, struct pictdb_file* db_file)
{
    size_t index = 0;
    int found = 0, errcode = 0;

    while(found == 0 && index < db_file -> header.max_files) {
        if(	1 == db_file -> metadata[index].is_valid &&
            !strncmp(&pict_id[0], &db_file -> metadata[index].pict_id[0], MAX_PIC_ID + 1)
          ) //if the image is valid and the the names match
            found = 1;
        else
            ++index;
    }

    if(found == 0) {
        errcode = ERR_FILE_NOT_FOUND;
    } else {
        if(db_file -> metadata[index].size[RES] == 0 || db_file -> metadata[index].offset[RES] == 0) {
            errcode = lazily_resize(RES, db_file, index); //si la résolution n'existe pas encore, on la crée
        }
        *image_size = db_file -> metadata[index].size[RES];
    }

    if(errcode == 0) {
        *image_buffer = calloc(*image_size, 1);
        if(image_buffer == NULL) {
            errcode = ERR_OUT_OF_MEMORY;
        } else {
            fseek(db_file -> fpdb, db_file -> metadata[index].offset[RES], SEEK_SET);
            if(1 != fread(*image_buffer, *image_size, 1, db_file -> fpdb)) {
                errcode = ERR_IO;
            }
            
        }
    }

    return errcode;
}
