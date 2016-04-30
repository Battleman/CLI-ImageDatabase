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
static int create_derivative(FILE* file, struct pictdb_header* header, struct pict_metadata* meta, int res, size_t* size_new, long* offset)
{
    fseek(file, meta->offset[RES_ORIG], SEEK_SET); //déplacement de la tête de lecture au début de l'image concernée

    /*Déclarations*/
    VipsImage* original;
    size_t size = sizeof(meta -> size[RES_ORIG]);
    void* buffer = NULL;

    fread(buffer, size_of_orig , 1, file); //on crée une image et on la lit, de la taille spécifiée
    if(0 != vips_jpegload_buffer(buffer, size, &original, NULL)) {
        return ERR_VIPS;
    }

    /*maintenant l'image est lue à travers un buffer, alors on la travaille*/
    VipsObject* process = VIPS_OBJECT( vips_image_new() );
    VipsImage** small = (VipsImage**) vips_object_local_array( process, 1 );
    double ratio = shrink_value(original, header -> res_resized[2*res], header -> res_resized[2*res + 1]);
    vips_resize(original, &small[0], ratio, NULL);

    /*la Vipsimage est modifiée, on l'écrit*/
    if(0 != vips_jpegsave_buffer(original, &buffer, &size_of_new, NULL)) { //size contient maintenant la taille de la petite image
        return ERR_VIPS;
    }
    *size_new = size; //on renvoie la nouvelle taille pour la mise à jour
    fseek(file, 0, SEEK_END); //déplacement en fin de fichier, pour écrire l'image redimensionnée
    long curr_pos = ftell(file); //getter de la position, pour l'écrire plus tard
    if(1 == fwrite(buffer, size_of_new, 1, file)) { //si l'écriture a réussi, on peut retourner la position
        *offset = curr_pos;
        return 0;
    }
    return ERR_IO; //sinon erreur
}

/**
 * 
 */
static int update_file(struct pictdb_file* file, int res, size_t index, size_t size, long deriv_offset)
{
    struct pictdb_header header = file -> header;
    header.db_version++;
    struct pict_metadata metadata = file -> metadata[index];
    metadata.size[res] = size; //taille de l'image redimensionnée
    metadata.offset[res] = deriv_offset; //la distance depuis le début du fichier

    int valid = 1;

    rewind(file -> fpdb); //retour au début pour se positionner sur le header
    valid &= fwrite(&header, sizeof(struct pictdb_header), 1, file -> fpdb); //réecriture du header
    fseek(file -> fpdb, index * sizeof(struct pictdb_header), SEEK_CUR); //déplacement à la bonne metadata
    valid &= fwrite(&metadata, sizeof(struct pict_metadata), 1, file -> fpdb); //overwrite de la metadata

    return valid ? 1 : ERR_IO;
}
int lazily_resize(int res, struct pictdb_file* file, size_t index)
{
    size_t size = 0, offset = 0, size = 0;


    /*Vérification des input*/
    if(res == RES_ORIG || file -> metadata[index].size[res] != 0) { //on ne fait rien si l'image a déjà été resized ou si on veut la resize à l'origine
        return 0;
    }
    if((res != RES_SMALL && res != RES_THUMB) || file == NULL || index < 0 || index > (file -> header.max_files)) {
        return ERR_INVALID_ARGUMENT;
    }

    int valid = create_derivative(file -> fpdb, &file -> metadata[index], res, &size); //size est modifié
    return (valid == 0) ? update_file(file, res, index, size, offset) : valid;

}
