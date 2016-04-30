#include "error.h"
#include "pictDB.h"
#include "image_content.h"

/**@brief Petit utilitaire pour retourner le ratio réduit d'une image
 *
 * @param image Pointeur vers une VipsImage, pour trouver ses dimensions
 * @param max_thumb_width Largeur maximale que peut avoir l'image redimensionnée
 * @param max_thumb_height Hauteur maximale que peut avoir l'image redimensionnée
 *
 * @return Le plus petit ratio entre une des dimension originale et sa dimension réduite
 */
static double
shrink_value(VipsImage* image, int max_thumb_width, int max_thumb_height)
{
    const double h_shrink = (double) max_thumb_width  / (double) image->Xsize ;
    const double v_shrink = (double) max_thumb_height / (double) image->Ysize ;
    return h_shrink > v_shrink ? v_shrink : h_shrink ;
}

/**@brief Crée une version réduite d'une image, et l'écrit en fin de fichier.
 *
 * @param file Le fichier dans lequel écrire l'image
 * @param header Le header de la base de donnée
 * @param meta La métadonnée de l'image concernée
 * @param res Le code de la résolution que doit avoir l'image redimensionnée
 * @param size_new Pointeur vers la taille qu'à la nouvelle image, pour mettre à jour la métadonnée
 * @param offset Pointeur vers taille du offset de l'image redimensionnée, pour mettre à jour la métadonnée
 *
 * @return 0 en cas de réussite, un code d'erreur sinon
 */
static int create_derivative(FILE* file, struct pictdb_header* header, struct pict_metadata* meta, int res, size_t* size_of_new, long* offset)
{
    fseek(file, meta->offset[RES_ORIG], SEEK_SET); //déplacement de la tête de lecture au début de l'image concernée

    /*Déclarations*/
    VipsImage* original;
    size_t size_of_orig = sizeof(meta -> size[RES_ORIG]);
    void* buffer = NULL;

    fread(buffer, size_of_orig , 1, file); //on crée une image et on la lit, de la taille spécifiée
    if(0 != vips_jpegload_buffer(buffer, size_of_orig, &original, NULL)) {
        return ERR_VIPS;
    }

    /*maintenant l'image est lue à travers un buffer, alors on la travaille*/
    VipsObject* process = VIPS_OBJECT( vips_image_new() );
    VipsImage** small = (VipsImage**) vips_object_local_array( process, 1 );
    double ratio = shrink_value(original, header -> res_resized[2*res], header -> res_resized[2*res + 1]);
    vips_resize(original, &small[0], ratio, NULL);

    /*la Vipsimage est modifiée, on l'écrit*/
    if(0 != vips_jpegsave_buffer(original, &buffer, size_of_new, NULL)) { //size contient maintenant la taille de la petite image
        return ERR_VIPS;
    }
    fseek(file, 0, SEEK_END); //déplacement en fin de fichier, pour écrire l'image redimensionnée
    long curr_pos = ftell(file); //getter de la position, pour l'écrire plus tard
    if(1 == fwrite(buffer, *size_of_new, 1, file)) { //si l'écriture a réussi, on peut retourner la position
        *offset = curr_pos;
        return 0;
    }
    return ERR_IO; //sinon erreur
}

/**@brief mise à jour de la métadonnée concernée et du header (overwrite des deux)
 *
 * @param db_file la base de donnée
 * @param res le code de la résolution de l'image redimensionnée
 * @param index L'index de la métadonnée concernée dans la base de donnée
 * @param size_resize Pointeur vers la taille (en octet) de l'image redimensionnée en fin de fichier
 * @param deriv_offset Pointeur vers distance entre le début du fichier et le début de l'image redimensionnée (offset)
 *
 * @return 0 en cas de succès, un code d'erreur sinon
 */
static int update_file(struct pictdb_file* db_file, int res, size_t index, size_t* size_resize, long* deriv_offset)
{
    struct pictdb_header header = db_file -> header;
    header.db_version++;
    struct pict_metadata metadata = db_file -> metadata[index];
    metadata.size[res] = *size_resize; //taille de l'image redimensionnée
    metadata.offset[res] = *deriv_offset; //la distance depuis le début du fichier


    rewind(db_file -> fpdb); //retour au début pour se positionner sur le header
    /*Déplacement à la bonne position et overwrite de la métadonnée*/
    if(	1 != fwrite(&header, sizeof(struct pictdb_header), 1, db_file -> fpdb) ||
		0 != fseek(db_file -> fpdb, index * sizeof(struct pict_metadata), SEEK_CUR) ||
		1 != fwrite(&metadata, sizeof(struct pict_metadata), 1, db_file -> fpdb)
	) { 
        return ERR_IO;
    }
    
    return 0;
}
/**@brief Effectue un redimensionnement d'une image, l'écrit à la fin et met à jour la métadonnée et le header concernés
 * 
 * @param res Le code de résolution de l'image redimensionnée
 * @param file
 */
int lazily_resize(int res, struct pictdb_file* db_file, size_t index)
{
    size_t size = 0, offset = 0;
    
    /*Vérification des input*/
    if((res != RES_ORIG && res != RES_SMALL && res != RES_THUMB) || db_file == NULL || index < 0 || index > (db_file -> header.max_files)) {
        return ERR_INVALID_ARGUMENT;
    }
    if(res == RES_ORIG || db_file -> metadata[index].size[res] != 0) { //on ne fait rien si l'image a déjà été resized ou si on veut la resize à l'origine
        return 0;
    }

    int valid = create_derivative(db_file -> fpdb, db_file -> header, &db_file -> metadata[index], res, &size); //size est modifié
    return (valid == 0) ? update_file(db_file, res, index, size, offset) : valid;

}
