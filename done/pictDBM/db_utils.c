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
 * Comparaison entre deux tableaux
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
    int errcode = 0;
    if(filename == NULL) {
        errcode = ERR_IO;
    }

    filename[MAX_PIC_ID + 11] = '\0';
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
        errcode = ERR_RESOLUTIONS;
        break;
    }

    return errcode;
}
/********************************************************************//**
 * Lecture d'une image.
 */
int read_disk_image(const char* filename, void** buffer, size_t* size)
{
    VipsObject* process = VIPS_OBJECT(vips_image_new());
    VipsImage** image = (VipsImage**) vips_object_local_array(process, 1);

    int errcode = 0;
    if(vips_jpegload(filename, image, NULL)) {
        errcode = ERR_VIPS;
    } else {
        if(vips_jpegsave_buffer(image[0], buffer, size, NULL)) {
            errcode = ERR_VIPS;
        }
    }
    return errcode;
}

/********************************************************************//**
 * Écriture d'une image
 */
int write_disk_image(FILE* file, const char* image, uint32_t image_size) {
	if(file == NULL || image_size == 0 || image == NULL || !strcmp(image, "")) return ERR_INVALID_ARGUMENT;
	rewind(file);
	if(image_size != fwrite(image, sizeof(char), image_size, file)) return ERR_IO;
	
	return 0;
}
/******************************************//**
 * File opening and header/metadata reading
 */
int do_open(const char* filename, const char* mode, struct pictdb_file* db_file)
{

    db_file -> fpdb = fopen(filename, mode);
    if(db_file->fpdb == NULL) return ERR_IO;
    if(1 != fread(&db_file -> header, sizeof(struct pictdb_header), 1, db_file -> fpdb))
        return ERR_IO;


    int max_files = (MAX_MAX_FILES > db_file -> header.max_files) ? db_file -> header.max_files : MAX_MAX_FILES; //selectionne le min entre MAX_MAX et le max spécifié

    if(NULL == (db_file -> metadata = calloc(max_files, sizeof(struct pict_metadata)))) {
        return ERR_OUT_OF_MEMORY;
    }
    if(db_file->header.max_files != fread(	db_file->metadata,
                                            sizeof(struct pict_metadata),
                                            db_file->header.max_files,
                                            db_file -> fpdb)) {
        return ERR_IO;
    }

    return 0;
}

/********************************************************************//**
 * Fermeture de fichier
 */
void do_close(struct pictdb_file* db_file)
{
    if(db_file == NULL) {
        fprintf(stderr, "ERR: impossible de fermer un fichier (base de donnée inexistante)");
    } else {
        free(db_file->metadata);
        if(db_file -> fpdb == NULL) {
            fprintf(stderr, "ERR: impossible de fermer le fichire fichier (non-ouvert ou inexistant)");
        } else {
            fclose(db_file -> fpdb);
        }
    }

}
