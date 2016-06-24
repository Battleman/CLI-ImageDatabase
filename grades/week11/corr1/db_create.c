/**
 * @file db_create.c
 * @brief pictDB library: do_create implementation.
 *
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <string.h> // for strncpy

/********************************************************************//**
 * Creates the database called db_filename. Writes the header and the
 * preallocated empty metadata array to database file.
 */
int do_create(const char* db_name, struct pictdb_file* db_file, uint32_t max_files, uint16_t thumb_res_X, uint16_t thumb_res_Y,
              uint16_t small_res_X, uint16_t small_res_Y)
{
    //Légère redondance des vérification, juste pour la robustesse de la méthode
    if(	db_name == NULL || !strcmp(db_name, "") || db_file == NULL ||
        max_files > MAX_MAX_FILES || max_files < 0 || thumb_res_X > MAX_THUMB_SIZE ||
        thumb_res_X < 0 || thumb_res_Y > MAX_THUMB_SIZE || thumb_res_Y < 0) {
        return ERR_INVALID_ARGUMENT;
    }
    db_file -> fpdb = fopen(db_name, "wb");
    struct pictdb_header header;
    int errcode = 0;

    if(NULL == db_file -> fpdb) {
        return ERR_IO;
    }

    //initialisation des valeurs par défaut
    strncpy	(header.db_name, CAT_TXT,  MAX_DB_NAME); //copie du nom par défaut
    (header).db_name[MAX_DB_NAME] = '\0'; //la "string" doit se terminer par \0

    header.db_version = 0;
    header.num_files = 0;
    header.max_files = max_files;

    header.res_resized[2 * RES_THUMB] = thumb_res_X;
    header.res_resized[2 * RES_THUMB + 1] = thumb_res_Y;
    header.res_resized[2 * RES_SMALL] = small_res_X;
    header.res_resized[2 * RES_SMALL + 1] = small_res_Y;

    db_file -> header = header;
    int printHead = 0;
    int printMeta = 0;
    //écriture du header
    if(1 != (printHead = fwrite(&db_file->header, sizeof(struct pictdb_header), 1, db_file->fpdb)))
        errcode = ERR_IO;

    //allocation et écriture des metadata
    if(	errcode == 0) {
        db_file -> metadata = calloc(max_files, sizeof(struct pict_metadata));
        if(db_file->metadata == NULL) errcode = ERR_OUT_OF_MEMORY;
        else if(max_files != (printMeta =  fwrite(db_file->metadata, sizeof(struct pict_metadata), max_files, db_file->fpdb))) errcode = ERR_IO;
    }

    printf("%d item(s) written\n", printHead+printMeta);

    do_close(db_file);

    return errcode; // retourne l'erreur;
}
