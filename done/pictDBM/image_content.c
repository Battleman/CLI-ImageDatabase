#include "error.h"
#include "pictDB.h"
#include "image_content.h"

int create_picture(FILE* file, struct pict_metadata* meta);
int update_file(struct pictdb_file* file, int res, size_t index, size_t size, size_t offset);

int lazily_resize(int res, struct pictdb_file* file, size_t index){
	size_t size = 0, offset = 0;
	
	if(res != RES_ORIG || res != RES_SMALL || res != RES_THUMB || file == NULL || index < 0 || index > MAX_MAX_FILES){
		return ERR_INVALID_ARGUMENT;
	}
	
	if(res == RES_ORIG){
		return 0;
	} else if(res == RES_SMALL){
		if(file -> metadata[index].size[res] != 0){
			return 0;
		}
		create_picture(file -> fpdb, &file -> metadata[index]);
	} else if(res == RES_THUMB){
		if(file -> metadata[index].size[res] != 0){
			return 0;
		}
		create_picture(file -> fpdb, &file -> metadata[index]);
	}
	
	return update_file(file, res, index, size, offset);
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
	
	return valid;
}
