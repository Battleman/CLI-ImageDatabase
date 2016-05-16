/**
 * @file dedup.h
 * @brief Header for de-duplication method.
 */

#include "pictDB.h"

/**
 * @brief Cherche et supprime les doublons d'une image
 *
 * Parcours la base de donnée pour chercher un doublon d'une image précise. Si un doublon est trouvé,
 * les valeurs du doublon sont mises dans celles de l'image recherchée. Si pas de
 * doublon n'est trouvé, l'offset de la résolution originale est mis à 0. La manière de penser
 * cette fonction est : vérifie à cet index si une telle image existe déjà. Si ce n'est pas le cas,
 * met le offset[RES_ORIG] à 0.
 *
 * @param db_file La DB dans laquelle chercher le doublon
 * @param index L'index de l'image de laquelle on cherche un doublon
 *
 * @return 0 en cas de succès, un code d'erreur sinon.
 */
int do_name_and_content_dedup(struct pictdb_file* db_file, uint32_t index);
