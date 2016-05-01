/* ** NOTE: undocumented in Doxygen
* @file db_utils.c
* @brief implementation of several tool functions for pictDB
*
* @author Mia Primorac
* @date 2 Nov 2015
*/

#include "pictDB.h"

#include <inttypes.h>
#include <stdint.h> // for uint8_t
#include <stdio.h> // for sprintf
#include <openssl/sha.h> // for SHA256_DIGEST_LENGTH

/********************************************************************//**
 * Human-readable SHA
 */
static void
sha_to_string (const unsigned char* SHA,
               char* sha_string)
{
    if (SHA == NULL) {
        return;
    }
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        sprintf(&sha_string[i*2], "%02x", SHA[i]);
    }

    sha_string[2*SHA256_DIGEST_LENGTH] = '\0';
}

/********************************************************************//**
 * pictDB header display.
 */

void print_header(const struct pictdb_header* header)
{
    printf("*****************************************\n**********DATABASE HEADER START**********\n");
    printf("DBNAME: %31s\n", header -> db_name);
    printf("VERSION: %" PRIu32 "\n", header -> db_version);
    printf("IMAGE COUNT: %" PRIu32 "\t\tMAX IMAGES: %" PRIu32 "\n", header -> num_files, header -> max_files);
    printf("THUMBNAIL: %" PRIu16 " x %" PRIu16 "\tSMALL: %" PRIu16 " x %" PRIu16 "\n", header -> res_resized[0],header -> res_resized[1],header -> res_resized[2],header -> res_resized[3]);
    printf("***********DATABASE HEADER END***********\n*****************************************\n");
}

/********************************************************************//**
 * Metadata display.
 */
void
print_metadata (const struct pict_metadata* metadata)
{
    char sha_printable[2*SHA256_DIGEST_LENGTH+1];
    sha_to_string(metadata -> SHA, sha_printable);

    printf("PICTURE ID: %31s\n", metadata -> pict_id);
    printf("SHA: %31s\n", sha_printable);
    printf("VALID: %" PRIu16 "\n", metadata -> is_valid);
    printf("UNUSED: %" PRIu16 "\n", metadata -> unused_16);
    printf("OFFSET ORIG.: %" PRIu64 "\t\tSIZE ORIG.: %" PRIu32 "\n", metadata -> offset[RES_ORIG], metadata -> size[RES_ORIG]);
    printf("OFFSET THUMB.: %" PRIu64 "\t\tSIZE THUMB.: %" PRIu32 "\n", metadata -> offset[RES_THUMB], metadata -> size[RES_THUMB]);
    printf("OFFSET SMALL : %" PRIu64 "\t\tSIZE SMALL : %" PRIu32 "\n", metadata -> offset[RES_SMALL], metadata -> size[RES_SMALL]);
    printf("ORIGINAL: %" PRIu32 " x %" PRIu32 "\n", metadata -> res_orig[0], metadata -> res_orig[1]);
    printf("*****************************************\n");
}
/* ces fonctions ne sont pas utilisées pour le moment, et sont probablement fausses, mais si
 * nécessaire, elles seront revues et décommentées. A ne pas lire/coriger pour le moment, merci :)
void copy_header(struct pictdb_header* copy, const struct pictdb_header* header){
	memcpy(&copy -> db_name, &header -> db_name, MAX_DB_NAME + 1);
	memcpy(&copy -> res_resized, &header -> res_resized, 2*(NB_RES - 1));
	copy -> db_version = header -> db_version;
	copy -> num_files = header -> num_files;
	copy -> max_files = header -> max_files;
	copy -> unused_32 = header -> unused_32;
	copy -> unused_64 = header -> unused_64;
}

void copy_metadata(struct pict_metadata* copy, const struct pict_metadata* metadata){
	memcpy(&copy -> pict_id, &metadata -> pict_id, MAX_PIC_ID + 1);
	memcpy(&copy -> SHA, &metadata -> SHA, SHA256_DIGEST_LENGTH);
	memcpy(&copy -> res_orig, &metadata -> res_orig, RES_ORIG);
	memcpy(&copy -> size, &metadata -> size, NB_RES);
	memcpy(&copy -> offset, &metadata -> offset, NB_RES);
	copy -> is_valid = metadata -> is_valid;
	copy -> unused_16 = metadata -> unused_16;
}*/

/**********************************
 * Remplacement du header
 */
int overwrite_header(FILE* file, struct pictdb_header* header)
{
    //Vérification des input
    if(file == NULL || header == NULL) {
        return ERR_INVALID_ARGUMENT;
    }

    long backup_position = ftell(file); //sauvegarde de la position actuelle
    int errcode = 0;
    rewind(file);
    if(1 != fwrite(header, sizeof(struct pictdb_header), 1, file)) {
        errcode = ERR_IO;
    }
    fseek(file, backup_position, SEEK_SET); //on revient de toute manière à la position avant l'appel
    return errcode;
}
/**********************************
 * Remplacement d'UNE métadonnée
 */
int overwrite_metadata(FILE* file, struct pict_metadata* metadata, size_t index)
{
    //Vérification des input
    if(file == NULL || metadata == NULL) {
        return ERR_INVALID_ARGUMENT;
    }
    int errcode = 0;
    long backup_position = ftell(file); //sauvegarde de la position actuelle
    fseek(file, sizeof(struct pictdb_header) + index * sizeof(struct pict_metadata), SEEK_SET);
    if(0 != errcode && 1 != fwrite(metadata, sizeof(struct pict_metadata), 1, file)) {
        errcode = ERR_IO;
    }
    fseek(file, backup_position, SEEK_SET); //on revient à la position avant l'appel
    return errcode;
}

/******************************************//**
 * File opening and header/metadata reading
 */
int do_open(const char* filename, const char* mode, struct pictdb_file* db_file)
{

    if( (db_file -> fpdb = fopen(filename, mode)) == NULL ||
        1 != fread(&db_file -> header, sizeof(struct pictdb_header), 1, db_file -> fpdb)) {
        return ERR_IO;
    }

    int max_files = (MAX_MAX_FILES > db_file -> header.max_files) ? db_file -> header.max_files : MAX_MAX_FILES; //selectionne le min entre MAX_MAX et le max spécifié

    if(NULL == (db_file -> metadata = calloc(max_files, sizeof(struct pict_metadata)))) {
        return ERR_OUT_OF_MEMORY;
    }

    int read_struct = 0;
    while(read_struct < max_files) {
        if(1 == fread(&(db_file->metadata[read_struct]), sizeof(struct pict_metadata), 1, db_file -> fpdb)) {
            ++read_struct;
        } else {
            return ERR_IO;
        }
    }


    return 0;
}

void do_close(struct pictdb_file* db_file)
{
    if(db_file == NULL || db_file -> fpdb == NULL) {
        fprintf(stderr, "ERR: impossible de fermer un fichier (null ou non-ouvert)");
    } else {
		free(db_file->metadata);
        fclose(db_file -> fpdb);
    }
}
