/* ** NOTE: undocumented in Doxygen
* @file db_utils.c
* @brief implementation of several tool functions for pictDB
*
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
 * Comparaison entre deux tableaux (permet entre autres la comparaison des SHA)
 */
int table_compare(unsigned char orig[], unsigned char comp[], size_t size)
{
    for(int i = 0; i < size; i++) {
        if(orig[i] != comp[i]) {
            return 1;
        }
    }
    return 0;
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
/**********************************
 * Remplacement du header
 */
int overwrite_header(FILE* file, struct pictdb_header* header)
{
    //Vérification des input
    if(file == NULL || header == NULL) {
        return ERR_INVALID_ARGUMENT;
    }

    int errcode = 0;
    rewind(file);
    if(1 != fwrite(header, sizeof(struct pictdb_header), 1, file)) {
        errcode = ERR_IO;
    }
    return errcode;
}
/**********************************
 * Remplacement d'UNE métadonnée
 */
int overwrite_metadata(struct pictdb_file* db_file, size_t index)
{
    //Vérification des input
    if (db_file == NULL || db_file->fpdb == NULL || db_file->metadata == NULL || index < 0 || index >= db_file->header.max_files) {
        return ERR_INVALID_ARGUMENT;
    }
    int errcode = 0;
    fseek(db_file->fpdb, sizeof(struct pictdb_header) + index * sizeof(struct pict_metadata), SEEK_SET);
    if(1 != fwrite(&(db_file->metadata[index]), sizeof(struct pict_metadata), 1, db_file->fpdb)) {
        errcode = ERR_IO;
    }
    return errcode;
}

/********************************************************************//**
 * Transformation de la dénomination de la résolution dans son code associé
 */
int resolution_atoi(const char* res_id)
{
    if(res_id == NULL) return -1;
    if(!strcmp(res_id, "small")) return RES_SMALL;
    if(!strcmp(res_id, "orig") || !strcmp(res_id, "original")) return RES_ORIG;
    if(!strcmp(res_id, "thumb") || !strcmp(res_id, "thumbnail")) return RES_THUMB;
    return -1;
}

/********************************************************************//**
 * "Concatène" des caractères pour former un nom selon nos standards
 */
int create_name(const char* pict_id, char* filename, int res)
{

    if(filename == NULL) {
        return ERR_IO;
    }

    filename[MAX_PIC_ID + 10] = '\0';
    strncpy(filename, pict_id, MAX_PIC_ID);
    switch(res) {
    case RES_THUMB:
        strcpy(&filename[strlen(pict_id)], "_thumb.jpg");
        break;
    case RES_SMALL:
        strcpy(&filename[strlen(pict_id)], "_small.jpg");
        break;
    case RES_ORIG:
        strcpy(&filename[strlen(pict_id)], "_orig.jpg");
        break;
    default:
        return ERR_RESOLUTIONS;
    }

    return 0;
}
/********************************************************************//**
 * Lecture d'une image (permet de remplir le buffer et déterminer la taille).
 */
int read_disk_image(const char* filename, void** buffer, size_t* size)
{
    VipsImage* in = vips_image_new_from_file(filename, NULL);
    if(in == NULL) return ERR_VIPS;
    if(vips_jpegsave_buffer(in, buffer, size, NULL)) return ERR_VIPS;
    return 0;
}

/********************************************************************//**
 * Écriture d'une image
 */
int write_disk_image(FILE* file, const char* image, uint32_t image_size)
{
    //Vérification des input
    if(file == NULL || image_size == 0 || image == NULL || !strcmp(image, "")) return ERR_INVALID_ARGUMENT;
    rewind(file); //retour au début (précaution)
    if(image_size != fwrite(image, sizeof(char), image_size, file)) return ERR_IO; //écriture

    return 0;
}

/********************************************************************//**
 * Parsing d'une chaîne de charactères
 */

void split(char* result[], char* tmp, const char* src, const char* delim, size_t len)
{
    //vérification des input
    if(tmp == NULL || src == NULL || result == NULL) return;
    strncpy(tmp, src, len);
    tmp = strtok(tmp, delim);

    for(int i = 0; i < MAX_QUERY_PARAM; ++i) {
        result[i] = tmp;
        tmp = strtok(NULL, delim);
    }
}

/******************************************//**
 * File opening and header/metadata reading
 */
int do_open(const char* filename, const char* mode, struct pictdb_file* db_file)
{

    db_file -> fpdb = fopen(filename, mode); //création d'une db
    if(db_file->fpdb == NULL) return ERR_IO;
    if(1 != fread(&db_file -> header, sizeof(struct pictdb_header), 1, db_file -> fpdb))
        return ERR_IO;


    int max_files = (MAX_MAX_FILES > db_file -> header.max_files) ? db_file -> header.max_files : MAX_MAX_FILES; //selectionne le min entre MAX_MAX et le max spécifié

    if(NULL == (db_file -> metadata = calloc(max_files, sizeof(struct pict_metadata)))) {
        return ERR_OUT_OF_MEMORY;
    }
    int read_file = 0;
    if(db_file->header.max_files != (read_file = fread(	db_file->metadata,
                                     sizeof(struct pict_metadata),
                                     db_file->header.max_files,
                                     db_file -> fpdb))) {
        return ERR_IO;
    }

    return 0;
}

/********************************************************************//**
 * Fermeture de fichier
 */
void do_close(struct pictdb_file* db_file)
{
    if(db_file != NULL) {
        free(db_file->metadata);
        if(db_file -> fpdb != NULL) {
            fclose(db_file -> fpdb);
        }
    }

}
