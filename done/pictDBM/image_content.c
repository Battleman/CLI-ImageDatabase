#include "error.h"
#include "pictDB.h"
#include "image_content.h"

int create_derivative(FILE* file, struct pictdb_header* header, struct pict_metadata* meta, int res, size_t* size_new);
int update_file(struct pictdb_file* file, int res, size_t index, size_t size, long deriv_offset);

int lazily_resize(int res, struct pictdb_file* file, size_t index){
	size_t size = 0, offset = 0, size = 0;
	
	if((res != RES_ORIG && res != RES_SMALL && res != RES_THUMB) || file == NULL || index < 0 || index > (file -> header.max_files)){
		return ERR_INVALID_ARGUMENT;
	}
	
	if(file -> metadata[index].size[res] != 0){
		return 0;
	} else {
		int valid = create_derivative(file -> fpdb, &file -> metadata[index], res, &size);
		if(valid != ERR_IO){
			return update_file(file, res, index, size, offset, valid);
		}
		return valid ? update_file(file, res, index, size, offset) : ERR_IO;
	}
}

int update_file(struct pictdb_file* file, int res, size_t index, size_t size, long deriv_offset){
	struct pictdb_header header = file -> header;
	header.db_version++;
	struct pict_metadata metadata = file -> metadata[index];
	metadata.size[res] = size;
	metadata.offset[res] = deriv_offset;
	
	
	int valid = 1;
	
	rewind(file -> fpdb);
	valid &= fwrite(&header, sizeof(struct pictdb_header), 1, file -> fpdb);
	fseek(file -> fpdb, index * sizeof(struct pictdb_header), SEEK_CUR);
	valid &= fwrite(&metadata, sizeof(struct pict_metadata), 1, file -> fpdb);
	
	return valid ? 1 : ERR_IO;
}

/**
 * @brief Computes the shrinking factor (keeping aspect ratio)
 *	
 * @author J.-C. Chappelier
 * 
 * @param image The image to be resized.
 * @param max_thumbnail_width The maximum width allowed for thumbnail creation.
 * @param max_thumbnail_height The maximum height allowed for thumbnail creation.
 */
double
shrink_value(VipsImage *image, int max_thumb_width, int max_thumb_height)
{
    const double h_shrink = (double) max_thumb_width  / (double) image->Xsize ;
    const double v_shrink = (double) max_thumb_height / (double) image->Ysize ;
    return h_shrink > v_shrink ? v_shrink : h_shrink ;
}

long create_derivative(FILE* file, struct pictdb_header* header, struct pict_metadata* meta, int res, size_t* size_new) {
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
		return curr_pos;
	}
	return ERR_IO; //sinon erreur
}
