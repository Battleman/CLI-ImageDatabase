/**
 * @file image_content.c
 * @brief Implémentation de méthodes de traitement d'images
 */

#include "error.h"
#include "pictDB.h"
#include "image_content.h"

/**@brief Petit utilitaire pour retourner le ratio réduit d'une image
 * 
 * 
  *@param image Pointeur vers une VipsImage, pour trouver ses dimensions
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


/**
 * @brief Helper method. Création et sauvegarde d'une version réduite d'une image.
 *
 * @param file Le fichier dans lequel lire et écrire l'image
 * @param meta La métadonnée dans laquelle travailler
 * @param header Le header de la DB
 * @param RES Code de la résolution dans laquelle transformer l'image.
 *
 * @return 0 en cas de succès, un code d'erreur sinon.
 */
static int create_small(FILE* file, struct pict_metadata* meta, struct pictdb_header* header, const int RES)
{
    size_t size_of_orig = meta->size[RES_ORIG]; //la taille à lire de l'image originale
    int errcode = 0;
    size_t size_of_small = 0;
    void* buffer_in = NULL;
    void* buffer_out = NULL;

    //Allocations nécessaires
    if(	NULL == (buffer_in = malloc(size_of_orig))) return ERR_OUT_OF_MEMORY;

    fseek(file, meta->offset[RES_ORIG], SEEK_SET); //déplacement au niveau de l'image originale
    if(1 != fread(buffer_in, size_of_orig, 1, file)) return ERR_IO; //lecture de l'image originale dans le buffer

    VipsObject* process = VIPS_OBJECT( vips_image_new() );
    VipsImage** image = (VipsImage**) vips_object_local_array( process, 1 );

    //On récupère l'image depuis le buffer
    if(vips_jpegload_buffer(buffer_in, size_of_orig, image, NULL)) errcode = ERR_VIPS;
    else {
        double ratio = shrink_value(*image, header->res_resized[2*RES], header->res_resized[2*RES+1]); //calcul du ratio
        VipsImage** image_small = (VipsImage**) vips_object_local_array( process, 1 ); //réceptacle pour l'image réduite

        vips_resize(image[0], &image_small[0], ratio, NULL); //redimensionnement de l'image

        if(vips_jpegsave_buffer(image_small[0], &buffer_out, &size_of_small, NULL)) errcode = ERR_VIPS;
        else {
            fseek(file, 0, SEEK_END); //déplacement en fin de fichier pour l'écriture
            meta->offset[RES] = ftell(file); //on sauve la position d'écriture
            meta->size[RES] = size_of_small;
            if(!fwrite(buffer_out, size_of_small, 1, file)) {
                errcode = ERR_IO;
            }
        }
    }
    g_object_unref(process);
    g_free(buffer_in);
    return errcode;
}

/**@brief helper method. Met à jour la metadata concernée.
 *
 * Mise à jour de la métadonnée modifiée par l'ajout d'une dimension.
 *
 * @param db_file La base de donnée (locale)
 * @param index l'index de la métadonnée modifiée
 *
 * @return 0 en cas de succès, un code d'erreur sinon.
 */
static int update_file(struct pictdb_file* db_file, size_t index)
{
    int errcode = 0;
    if(0 != (errcode = overwrite_metadata(db_file, index))) {
        errcode = ERR_IO;
    }
    return errcode;
}
/*****************************************
 * Redimensionnement d'une image
 ******/
int lazily_resize(const int RES, struct pictdb_file* db_file, size_t index)
{

    /*Vérification des input*/
    if(RES < 0 || RES >= NB_RES) 									return ERR_RESOLUTIONS;
    if(db_file == NULL || db_file->fpdb == NULL) 					return ERR_INVALID_ARGUMENT;
    if(index < 0 || index >= db_file->header.max_files) 			return ERR_INVALID_PICID;
    if(RES == RES_ORIG || db_file->metadata[index].size[RES] != 0) 	return 0; //si on cherche la RES_ORIG, rien à faire

    int errcode = 0;
    if(0 != (errcode = create_small(db_file->fpdb, &db_file->metadata[index], &db_file->header, RES))) return errcode;
    return update_file(db_file, index);
}

/*****************************************
 * Getter de résolution d'une image
 ******/
int get_resolution(uint32_t* height, uint32_t* width, const char* image_buffer, size_t image_size)
{
    int errcode = 0;
    VipsObject* process = VIPS_OBJECT( vips_image_new() );
    VipsImage** image = (VipsImage**) vips_object_local_array( process, 1 );

	//vérification de l'obtention d'une image valide à partir du buffer
    if(vips_jpegload_buffer((char*)image_buffer, image_size, image, NULL)) {
        errcode = ERR_VIPS;
    } else {
		//permet d'obtenir les tailles des images
        *width = (*image)->Xsize;
        *height = (*image)->Ysize;
    }

    g_object_unref(process);

    return errcode;
}
