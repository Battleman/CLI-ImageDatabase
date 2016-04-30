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
/******************************************//**
 * File opening and header/metadata reading
 */
int do_open(const char* filename, const char* mode, struct pictdb_file* db_file)
{

    if((db_file -> fpdb = fopen(filename, mode)) == NULL) {
        return ERR_IO;
    }

    if(1 != fread(&db_file -> header, sizeof(struct pictdb_header), 1, db_file -> fpdb)) {
        return ERR_IO;
    }

    int max_files = (MAX_MAX_FILES > db_file -> header.max_files) ? db_file -> header.max_files : MAX_MAX_FILES;

    if(NULL == (db_file -> metadata = calloc(max_files, sizeof(struct pict_metadata)))) {
        return  ERR_OUT_OF_MEMORY;
    }

    int read_struct = 0;
    struct pict_metadata temp;
    while(read_struct < db_file -> header.max_files) {
        if(1 == fread(&temp, sizeof(struct pict_metadata), 1, db_file -> fpdb)) {
            db_file -> metadata[read_struct] = temp;
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
        fclose(db_file -> fpdb);
    }
}
