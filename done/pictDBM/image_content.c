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


static int create_small(FILE* file, struct pict_metadata* meta, struct pictdb_header* header, const int RES)
{
    size_t size_of_orig = meta->size[RES_ORIG]; //la taille à lire de l'image originale
    int errcode = 0;
    size_t* size_of_small = NULL;
    void* buffer_in = NULL;
    void** buffer_out = NULL;
    //Allocations nécessaires
    if (NULL == (size_of_small= malloc(sizeof(size_t))) || //ou on aura la taille de l'image réduite
        NULL == (buffer_in = malloc(size_of_orig)) || //le buffer read l'image
        NULL == (buffer_out = malloc(sizeof(void*)))) { //le buffer depuis lequel write l'image
        return ERR_OUT_OF_MEMORY;
    }

    fseek(file, meta->offset[RES_ORIG], SEEK_SET); //déplacement au niveau de l'image originale
    fread(buffer_in, size_of_orig, 1, file); //lecture de l'image originale dans le buffer

    VipsObject* process = VIPS_OBJECT( vips_image_new() );
    VipsImage** image = (VipsImage**) vips_object_local_array( process, 1 );

    //On récupère l'image depuis le buffer
    if(vips_jpegload_buffer(buffer_in, size_of_orig, image, NULL)) {
        errcode = ERR_VIPS;
    } else {
        double ratio = shrink_value(*image, header->res_resized[2*RES], header->res_resized[2*RES+1]);

        vips_resize(image[0], &image[0], ratio, NULL); //redimensionnement de l'image
        if(vips_jpegsave_buffer(image[0], buffer_out, size_of_small, NULL)) {
            errcode = ERR_VIPS;
        } else {
            fseek(file, 0, SEEK_END); //déplacement en fin de fichier pour l'écriture
            meta->offset[RES] = ftell(file); //on sauve la position d'écriture
            meta->size[RES] = *size_of_small;
            if(!fwrite(*buffer_out, *size_of_small, 1, file)) {
                errcode = ERR_IO;
            }
        }
    }
    g_object_unref(process);
    g_free(size_of_small);
    g_free(buffer_in);
    g_free(buffer_out);
    return errcode;
}

static int update_file(struct pictdb_file* db_file, size_t index)
{
    int errcode = 0;
    db_file->header.db_version++;
    if(	0 != (errcode = overwrite_header(db_file->fpdb, &db_file->header)) ||
        0 != (errcode = overwrite_metadata(db_file->fpdb, &db_file->metadata[index], index)) ) {
        errcode = ERR_IO;
    }
    return errcode;
}

/**@brief Effectue un redimensionnement d'une image, l'écrit à la fin et met à jour la métadonnée et le header concernés
 *
 * @param res Le code de résolution de l'image redimensionnée
 * @param db_file la base de donnée dans laquelle travailler
 * @param index L'index de l'image (metadonnée) à redimensionner
 */
int lazily_resize(const int RES, struct pictdb_file* db_file, size_t index)
{

    /*Vérification des input*/
    if(RES < 0 || RES >= NB_RES) {
        return ERR_RESOLUTIONS;
    }
    if(db_file == NULL || db_file->fpdb == NULL) {
        return ERR_INVALID_ARGUMENT;
    }
    if(index < 0 || index >= db_file->header.max_files) {
        return ERR_INVALID_PICID;
    }
    if(RES == RES_ORIG || db_file->metadata[index].size[RES] != 0) {
        return 0;
    }
    int errcode = 0;
    if(	0 != (errcode = create_small(db_file->fpdb, &db_file->metadata[index], &db_file->header, RES)) ||
        0 != (errcode = update_file(db_file, index))) {
        return errcode;
    }
    return 0;
}


