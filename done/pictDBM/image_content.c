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

int lazily_resize(const int RES, struct pictdb_file* db_file, size_t index) {
	
	if(RES < 0 || RES >= NB_RES) {
		return ERR_RESOLUTIONS;
	}
	if(db_file == NULL || db_file->fpdb == NULL) {
		return ERR_INVALID_ARGUMENT;
	}
	struct pictdb_header header = db_file->header;
	if(index < 0 || index >= header.max_files) {
		return ERR_INVALID_PICID;
	}
	
	int errcode = 0;
	struct pict_metadata meta = db_file->metadata[index];
	if(RES == RES_ORIG || meta.size[RES] == 0) {
		FILE* file = db_file->fpdb;
		size_t* size_of_small = NULL;
		void* buffer_in = NULL;
		void** buffer_out = NULL;
		
			
		size_t size_of_orig = meta.size[RES_ORIG]; //la taille à lire de l'image originale
		//Allocations nécessaires
		if (NULL == (size_of_small= malloc(sizeof(size_t))) || //où on aura la taille de l'image réduite
			NULL == (buffer_in = malloc(size_of_orig)) || //le buffer read l'image
			NULL == (buffer_out = malloc(sizeof(void*)))) { //le buffer depuis lequel write l'image
				errcode = ERR_OUT_OF_MEMORY;
		} else {
		
			fseek(file, meta.offset[RES_ORIG], SEEK_SET); //déplacement au niveau de l'image originale
			fread(buffer_in, size_of_orig, 1, file);
			VipsObject* process = VIPS_OBJECT( vips_image_new() );
			VipsImage** image = (VipsImage**) vips_object_local_array( process, 1 );
			
			
			//On récupère l'image dans le buffer_in
			if(vips_jpegload_buffer(buffer_in, size_of_orig, image, NULL)) {
				errcode = ERR_VIPS;
			} else {
			
				double ratio = shrink_value(*image, header.res_resized[2*RES], header.res_resized[2*RES+1]);
			
				vips_resize(image[0], &image[0], ratio, NULL);
				if(vips_jpegsave_buffer(image[0], buffer_out, size_of_small, NULL)) {
					errcode = ERR_VIPS;
				} else {
					fseek(file, 0, SEEK_END); //déplacement en fin de fichier pour l'écriture
					meta.offset[RES] = ftell(file); //on sauve la position d'écriture
					meta.size[RES] = *size_of_small; 
					if(!fwrite(*buffer_out, *size_of_small, 1, file)) {
						errcode = ERR_IO;
					} else {
						header.db_version++;
						rewind(file);
						fwrite(&header, sizeof(struct pictdb_header), 1, file);
						fseek(file, index*sizeof(struct pict_metadata), SEEK_CUR);
						fwrite(&meta, sizeof(struct pict_metadata), 1, file);
					}	
				}
			}
			g_object_unref(process);
		}
		
		g_free(buffer_in);
		g_free(buffer_out);
	}
	return errcode;
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
/*static int create_derivative(FILE* file, struct pictdb_header* header, struct pict_metadata* meta, int res, size_t* size_of_new, uint64_t* offset)
{
    fseek(file, meta->offset[RES_ORIG], SEEK_SET); //déplacement de la tête de lecture au début de l'image concernée

    //Déclarations
    VipsObject* process = VIPS_OBJECT( vips_image_new() );
    VipsImage** curr_image = (VipsImage**) vips_object_local_array( process, 1 );
    size_t size_of_orig = meta->size[RES_ORIG]; //la taille à lire de l'image originale
    void* buffer_lect = malloc(size_of_orig);
	void** buffer_writ = malloc(sizeof(void*));
	
	//Lecture de l'image vers le buffer de lecture
    fread(buffer_lect, size_of_orig , 1, file); //on crée une image et on la lit, de la taille spécifiée
    //FILE* myfile = fopen("NewFile", "w");
    //fwrite(buffer_lect, size_of_orig, 1, myfile);
    //fclose(myfile);
    if(0 != vips_jpegload_buffer(buffer_lect, size_of_orig, curr_image, NULL)) {
        return ERR_VIPS;
    }

    //maintenant l'image est lue à travers un buffer, alors on la travaille
    double ratio = shrink_value(curr_image[0], header -> res_resized[2*res], header -> res_resized[2*res + 1]);
    vips_resize(curr_image[0], &curr_image[0], ratio, NULL);

    //la Vipsimage est modifiée, on l'écrit
    if(0 != vips_jpegsave_buffer(curr_image[0], buffer_writ, size_of_new, NULL)) { //size contient maintenant la taille de la petite image
        return ERR_VIPS;
    }
    fseek(file, 0, SEEK_END); //déplacement en fin de fichier, pour écrire l'image redimensionnée
    uint64_t curr_pos = ftell(file); //getter de la position, pour l'écrire plus tard
    if(!fwrite(*buffer_writ, sizeof(char), *size_of_new, file)) { //si l'écriture a réussi, on peut retourner la position
        *offset = curr_pos;
        free(buffer_lect);
        return 0;
    }
    return ERR_IO; //sinon erreur
}*/

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
/*static int update_file(struct pictdb_file* db_file, int res, size_t index, size_t* size_resize, uint64_t* deriv_offset)
{
    struct pictdb_header header = db_file -> header;
    header.db_version++;
    struct pict_metadata metadata = db_file -> metadata[index];
    metadata.size[res] = *size_resize; //taille de l'image redimensionnée
    metadata.offset[res] = *deriv_offset; //la distance depuis le début du fichier


    rewind(db_file -> fpdb); //retour au début pour se positionner sur le header
    //déplacement à la bonne position et overwrite de la métadonnée
    if(!fwrite(&header, sizeof(struct pictdb_header), 1, db_file -> fpdb) ||
        0 != fseek(db_file -> fpdb, index * sizeof(struct pict_metadata), SEEK_CUR) ||
        0 == fwrite(&metadata, sizeof(struct pict_metadata), 1, db_file -> fpdb)
      ) {
        return ERR_IO;
    }

    return 0;
}*/
/**@brief Effectue un redimensionnement d'une image, l'écrit à la fin et met à jour la métadonnée et le header concernés
 *
 * @param res Le code de résolution de l'image redimensionnée
 * @param file
 */
 
/*int lazily_resize(int res, struct pictdb_file* db_file, size_t index)
{
    size_t* size = malloc(sizeof(size_t));
    uint64_t offset = 0;

    //Vérification des input
    if((res != RES_ORIG && res != RES_SMALL && res != RES_THUMB) || db_file == NULL || index < 0 || index > (db_file -> header.max_files)) {
        return ERR_INVALID_ARGUMENT;
    }
    if(res == RES_ORIG || db_file -> metadata[index].size[res] != 0) { //on ne fait rien si l'image a déjà été resized ou si on veut la resize à l'origine
        return 0;
    }

    int valid = create_derivative(db_file -> fpdb, &db_file -> header, &db_file -> metadata[index], res, size, &offset); //size est modifié
    return (valid == 0) ? update_file(db_file, res, index, size, &offset) : valid;

}*/

