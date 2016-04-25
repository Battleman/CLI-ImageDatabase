#include "error.h"
#include "pictDB.h"
#include "image_content.h"

int create_derivative(FILE* file, struct pict_metadata* meta, int res);
int update_file(struct pictdb_file* file, int res, size_t index, size_t size, size_t offset);

int lazily_resize(int res, struct pictdb_file* file, size_t index){
	size_t size = 0, offset = 0;
	
	if((res != RES_ORIG && res != RES_SMALL && res != RES_THUMB) || file == NULL || index < 0 || index > (file -> header.max_files)){
		return ERR_INVALID_ARGUMENT;
	}
	
	if(file -> metadata[index].size[res] != 0){
		return 0;
	} else {
		int valid = create_derivative(file -> fpdb, &file -> metadata[index], res);
		return valid ? update_file(file, res, index, size, offset) : ERR_IO;
	}
}

int update_file(struct pictdb_file* file, int res, size_t index, size_t size, size_t offset){
	struct pictdb_header header = file -> header;
	header.db_version++;
	struct pict_metadata metadata = file -> metadata[index];
	metadata.size[res] = size;
	metadata.offset[res] = offset;
	
	
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

int create_derivative(FILE* file, struct pict_metadata* meta, int res) {
	fseek(file, meta->offset[RES_ORIG], SEEK_SET); //déplacement de la tête de lecture au début de l'image concernée
	VipsImage* original;
	size_t size_of_orig= sizeof(meta->size[RES_ORIG]);
	void* buffer = NULL;
	fread(buffer, size_of_orig , 1, file); //on crée une image et on la lit, de la taille spécifiée
	if(0 != vips_jpegload_buffer(buffer, size_of_orig, &original, NULL)) {
		return ERR_VIPS;
	} 
	//maintenant l'image est lue à travers un buffer, alors on la travaille
    VipsObject* process = VIPS_OBJECT( vips_image_new() );
    VipsImage** small = (VipsImage**) vips_object_local_array( process, 1 );
    double ratio = shrink_value(original, meta->size[res], meta->size[res]);
	
	vips_resize(original, &small[0], ratio, NULL);
	//la VIpsimage est modifiée, on l'écrit
	size_t size_of_new = sizeof(meta->size[res]);
	if(0 != vips_jpegsave_buffer(original, &buffer, &size_of_new, NULL)) {
		return ERR_VIPS;
	}
	fseek(file, meta->offset[res], SEEK_SET);
	fwrite(buffer, size_of_new, 1, file);
	return 0;
}
