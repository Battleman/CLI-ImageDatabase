/**
 * @file pictDB.h
 * @brief Main header file for pictDB core library.
 *
 * Defines the format of the data structures that will be stored on the disk
 * and provides interface functions.
 *
 * The picture database starts with exactly one header structure
 * followed by exactly pictdb_header.max_files metadata
 * structures. The actual content is not defined by these structures
 * because it should be stored as raw bytes appended at the end of the
 * database file and addressed by offsets in the metadata structure.
 *
 * @author Mia Primorac
 * @date 2 Nov 2015
 */

#ifndef PICTDBPRJ_PICTDB_H
#define PICTDBPRJ_PICTDB_H

#include "error.h" /* not needed here, but provides it as required by
                    * all functions of this lib.
                    */
#include <stdio.h> // for FILE
#include <stdint.h> // for uint32_t, uint64_t
#include <openssl/sha.h> // for SHA256_DIGEST_LENGTH
#include <string.h>
#include <stdlib.h>
#include <vips/vips.h>

#define CAT_TXT "EPFL PictDB binary"

/* constraints */
#define MAX_DB_NAME 31  // max. size of a PictDB name
#define MAX_PIC_ID 127  // max. size of a picture id
#define MAX_MAX_FILES 100000  // max. size of the db
#define MAX_THUMB_SIZE 128
#define MAX_SMALL_SIZE 512

/* For is_valid in pictdb_metadata */
#define EMPTY 0
#define NON_EMPTY 1

// pictDB library internal codes for different picture resolutions.
#define RES_THUMB 0
#define RES_SMALL 1
#define RES_ORIG  2
#define NB_RES    3

/*Amount of defined commands*/
#define NB_COMMANDS 6

/*Amount of maximum arguments of a URI function */
#define MAX_QUERY_PARAM 5

#ifdef __cplusplus
extern "C" {
#endif


struct pictdb_header {
    char db_name[MAX_DB_NAME + 1]; //!<Nom de la base de donnée
    uint32_t db_version; //!< Version de la base de donnée (modif à chaque insertion/suppression d'image originale)
    uint32_t num_files; //!<Nombre de fichiers actuellement dedans
    uint32_t max_files; //!<Nombre maximum de fichiers autorisés
    uint16_t res_resized[2*(NB_RES - 1)]; //!<Dimensions des résolutions inférieurs
    uint32_t unused_32; //!<Inutilisé
    uint64_t unused_64; //!<Inutilisé
};

struct pict_metadata {
    char pict_id[MAX_PIC_ID + 1]; //!<Nom de l'image
    unsigned char SHA[SHA256_DIGEST_LENGTH]; //!<SHA de l'image
    //de taille 2, car toujours 2 dimensions (on va pas réinventer la roue)
    uint32_t res_orig[2];//!<Dimensions de la résolution originale
    uint32_t size[NB_RES]; //!<Taille (poids) de l'image dans ses résolutions
    uint64_t offset[NB_RES]; //!<Offset (distance au début du disque) de chaque résolution
    uint16_t is_valid; //!<"Interrupteur" de validité de l'image
    uint16_t unused_16; //!<Inutilisé
};

struct pictdb_file {
    FILE* fpdb; //!<Lien vers le fichier de la DB
    struct pictdb_header header; //!<Un header
    struct pict_metadata* metadata; //!<Et un tableau de métadonnées
};

enum do_list_mode {STDOUT, JSON};


/**
 * @brief Prints database header informations.
 *
 * @param header The header to be displayed.
 */
void print_header(const struct pictdb_header* header);


/**
 * @brief Prints picture metadata informations.
 *
 * @param metadata The metadata of one picture.
 */
void print_metadata (const struct pict_metadata* metadata);

/**
 * @brief Displays (on stdout) pictDB metadata.
 *
 * @param myfile In memory structure with header and metadata.
 * @param mode The behavious mode. STDOUT (just print) or JSON.
 * 
 * @return If mode STDOUT, NULL. If mode JSON, a JSON string of the database, or NULL in case of an error.
 */

const char* do_list(const struct pictdb_file* myfile, enum do_list_mode mode);


/**
 * @brief Creates the database called db_filename. Writes the header and the
 *        preallocated empty metadata array to database file.
 *
 * @param db_file In memory structure with header and metadata.
 */
int do_create(const char* filename, struct pictdb_file* db_file, uint32_t max_files, uint16_t thumb_res_X, uint16_t thumb_res_Y,
              uint16_t small_res_X, uint16_t small_res_Y);

/**
 * @brief opens a file in the desired mode, and stocks the read file (header+metadata table)
 *
 * @param filename name of the file to open
 * @param mode opening (r/w (b)...)
 * @param db_file In memory structure ; where to stock the read file
 *
 * @return 0 if success; a non-null integer if fail (corresponds to an
 * 			error code, defined in error.h
 **/
int do_open(const char* filename, const char* mode, struct pictdb_file* db_file);

/**
 * @brief Closes a file (if possible)
 *
 * @param db_file The file to close
 */

void do_close(struct pictdb_file* db_file);

/**
 * @brief Deletes a specified entry in the database of the specified file
 *
 * @param picname The ID of the pic to be deleted
 * @param file The file (already opened) in which delete the image
 *
 * @return 0 if success; else a non-null integer (corresponds to an
 * 			error code defined in error.h)
 **/
int do_delete(const char* picname, struct pictdb_file* file);

/**
 * @brief Remplacement du header (par overwrite) sur le fichier spécifié. N'influence pas la tête de lecture.
 *
 * @param file Le fichier sur lequel remplacer le header
 * @param header Le header à réécrire
 *
 * @return 0 en cas de succès, un code d'erreur sinon
 */
int overwrite_header(FILE* file, struct pictdb_header* header);

/**
 * @brief Remplacement d'une metadata (par overwrite) à un index spécifié sur le fichier spécifié. N'influence pas la tête de lecture.
 *
 * @param file Le fichier sur lequel remplacer la métadonnée
 * @param metadata La métadonnée à utiliser pour le remplacement
 * @param index L'index dans le fichier de la métadonnée à remplacer
 *
 * @return 0 en cas de succès, un code d'erreur sinon
 */
int overwrite_metadata(struct pictdb_file* db_file, size_t index);

/**
 * @brief "Change" un texte (comme option de commande) en son code de résolution asosicé
 *
 * e.g. change "small" dans la constante RES_SMALL.
 *
 * @param res_id Le texte de la résolution
 *
 * @return Le code de résolution de l'image (défini dans ce fichier) ou -1 en cas d'erreur
 */
int resolution_atoi(const char* res_id);

/**
 * @brief Lecture d'une résolution d'une image
 *
 * Cherche une image dans la base de donnée. Si la résolution souhaitée n'existe pas,
 * les fonctions nécessaires sont appelées pour la créer. La l'image est alors
 * retournée dans un argument de sortie.
 *
 * @param pict_id Le nom de l'image
 * @param RES La résolution souhaitée de l'image
 * @param image_buffer Paramètre de sortie : Buffer dans lequelle stocker l'image
 * @param image_size Paramètre de sortie : taille de l'image
 * @param db_file La base de donnée
 *
 * @return 0 en cas de succès, un code d'erreur sinon
 */
int do_read(const char pict_id[], const int RES, char** image_buffer, uint32_t* image_size, struct pictdb_file* db_file);

/**
 * @brief compare les @p size  premières entrées de deux tableaux
 *
 * Les deux tableaux doivent contenir des char (e.g. comparaison de SHA). @p Size doit
 * être plus petit que la taille du plus petit tableau. Si les tailles sont différentes,
 * aucune erreur n'est retournée.
 * @param orig Le premier tableau à comparer
 * @param comp Le second tableau à comparer
 * @param size La taille (normalement des tableaux) sur laquelle comparer les tableaux
 *
 * @return 0 en cas de succès, un code d'erreur sinon.
 */
int table_compare(unsigned char orig[], unsigned char comp[], size_t size);

/** @brief Insère une image dans la base de donnée
 *
 * @param pict_id Le nom de l'image
 * @param img L'image
 * @param size La taille de l'image
 * @param db_file La DB dans laquelle insérer l'image
 *
 * @return 0 en cas de succès, un code d'erreur sinon.
 */
int do_insert(const char pict_id[], char* img, size_t size, struct pictdb_file* db_file);

/**@brief Crée un nom standardisé pour une image
 *
 * @param pict_id Le nom de l'image
 * @param filename L'emplacement pour y écrire le nom
 * @param res Le code de la résolution
 *
 * @return 0 en cas de succès, un code d'erreur sinon
 */
int create_name(const char* pict_id, char* filename, int res);

/**@brief lit une image sur le disque
 *
 * Lit sur le disque une image de type jpeg et la renvoie dans un argument de sortie
 *
 * @param filename Le nom à donner à l'image
 * @param buffer L'emplacement dans lequelle mettre l'image
 * @param size La taille de l'image
 *
 * @return 0 en cas de succès, un code d'erreur sinon
 */
int read_disk_image(const char* filename, void** buffer, size_t* size);

/**@brief Écrit une image sur le disque
 *
 * Cherche une image dans la base de donnée (dans la résolution choisie) et l'écrit sur
 * le disque sous la formed'une image au format jpeg
 *
 * @param file Le fichier image (ouvert)
 * @param image L'image à écrire dans le fichier
 * @param image_size La taille de l'image
 *
 * @return 0 en cas de succès, un code d'erreur sinon.
 */
int write_disk_image(FILE* file, const char* image, uint32_t image_size);

/**@brief Parse une chaîne de charactères
 *
 * Permet de récupérer tous les arguments URI reçus
 *
 * @param result pointeur vers le tableau de charactères renvoyant les différents arguments
 * @param tmp chaîne de charactères contenant la chaîne intermédiaire parsée avec des '/0'
 * @param src chaîne de charactères originales contenant tous les arguments en format URI
 * @param delim délimiteurs de charactères
 * @param len longueur de la chaîne à étudier
 *
 * @return 0 en cas de succès, un code d'erreur sinon.
 */
void split(char* result[], char* tmp, const char* src, const char* delim, size_t len);

#ifdef __cplusplus
}
#endif
#endif
