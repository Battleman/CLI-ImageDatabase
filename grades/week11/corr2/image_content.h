/**
 * @file image_content.h
 *
 * @brief entête des méthodes qui traitent des images
 */

#include "pictDB.h"

/**
 * @brief Redimensionne une image
 *
 * Modifie une image selon le code de résolution fourni. Utilisation de VIPS
 *
 * @param res Le code de la résolution
 * @param db_file La DB dans laquelle chercher et écrire l'image
 * @param index L'index de l'image à redimensionner
 *
 * @return 0 en cas de succès, un code d'erreur sinon.
 */
int lazily_resize(const int res, struct pictdb_file* db_file, size_t index);

/**
 * @brief Getter de la résolution d'une image
 *
 * @param height Localisaton où écrire la hauteur
 * @param width Localisaton où écrire la largeur
 * @param image_buffer Buffer dans lequel est contenu l'image
 * @param size La taille (en octet)de l'image
 *
 * @return 0 en cas de succès, code d'erreur sinon
 *
 */
int get_resolution(uint32_t* height, uint32_t* width, const char* image_buffer, size_t image_size);
